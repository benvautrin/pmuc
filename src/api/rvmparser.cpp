/*
 * Plant Mock-Up Converter
 *
 * Copyright (c) 2013, EDF. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301  USA
 */

#include "rvmparser.h"
#include "rvmprimitive.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdio>
#include <cassert>
#include <algorithm>
#include <array>
#include <stdint.h>

#include "rvmreader.h"

using namespace std;

#ifdef _WIN32
#define PATHSEP '\\'
#else
#define PATHSEP '/'
#endif

/**
 * List of all RVM keywords used inside the RVM format
 */
static const std::vector<const char*> RVM_KEYWORDS = {
    "HEAD",
    "END",
    "MODL",
    "CNTB",
    "PRIM",
    "CNTE",
    "COLR"
};

typedef std::vector<std::vector<std::vector<std::pair<Vector3F, Vector3F>>>>        FacetGroup;

union Primitive
{
    Primitives::Box                 box;
    Primitives::Pyramid             pyramid;
    Primitives::RectangularTorus    rTorus;
    Primitives::CircularTorus       cTorus;
    Primitives::EllipticalDish      eDish;
    Primitives::SphericalDish       sDish;
    Primitives::Snout               snout;
    Primitives::Cylinder            cylinder;
    Primitives::Sphere              sphere;
};

/**
 * Identifier class
 */

struct Identifier
{
    inline bool operator==(const char* rhs) const
    {
        return chrs[0] == rhs[0] && chrs[1] == rhs[1]
            && chrs[2] == rhs[2] && chrs[3] == rhs[3];
    }

    inline bool operator!=(const char* rhs) const
    {
        return chrs[0] != rhs[0] || chrs[1] != rhs[1]
            || chrs[2] != rhs[2] || chrs[3] != rhs[3];
    }

    inline bool empty() const
    {
        return *chrs == 0;
    }

    bool isValid() const
    {
        for(const auto &keyword : RVM_KEYWORDS)
            if(*this == keyword)
                return true;

        return false;
    }

    inline std::string toString() const
    {
        return std::string(chrs, chrs + 4);
    }

    char        chrs[4];
};

///////////////////////////////////
//// Multiple read functions
///////////////////////////////////

template<typename T>
inline T& read_(std::istream& in, T& value)
{
    char *charPtr = reinterpret_cast<char*>(&value);
    for (size_t i = 0; i < sizeof(T); ++i)
        charPtr[sizeof(T) - 1 - i] = in.get();

    return value;
}

template<typename T>
inline T read_(std::istream& in)
{
    T value;
    read_(in, value);

    return value;
}

template<>
inline Identifier& read_<Identifier>(std::istream& in, Identifier& res)
{
    static char buf[12];
    auto chrs = res.chrs;

    // read the first 12 bytes and extract the first 3 characters
    in.read(buf, 12);
    {
        char* ptr = buf;

        for (int i = 0; i < 3; ++i, ptr += 4)
        {
            // the first three bytes of the current double word have to be zero
            if (ptr[0] != 0 || ptr[1] != 0 || ptr[2] != 0)
            {
                *chrs = 0;
                return res;
            }

            // extract character
            chrs[i] = ptr[3];
        }
    }

    // check if we have the end identifier
    if (chrs[0] == 'E' && chrs[1] == 'N' && chrs[2] == 'D')
        chrs[3] = 0;
    else
    {
        in.read(buf, 4);
        if (buf[0] != 0 || buf[1] != 0 || buf[2] != 0)
        {
            *chrs = 0;
            return res;
        }

        chrs[3] = buf[3];
    }

    return res;
}

template<>
string& read_<string>(istream& is, string& str)
{
    const unsigned int size = read_<unsigned int>(is) * 4;
    if (size == 0)
    {
        str.empty();
        return str;
    }

#ifndef ICONV_FOUND

    str.resize(size);
    unsigned int rs = 0;
    for(unsigned int i = 0; i < size; ++i) {
        str[i] = is.get();
        if(str[i] == 0 && !rs) {
            rs = i;
        }
    }
    if(rs) {
        str.resize(rs);
    }

#else

    char buffer[1024];
    buffer[0] = 0;
    is.read(buffer, size);
    buffer[size] = 0;

    // If already in UTF-8, no change
    if (m_cd == (iconv_t)-1)
    {
        str = buffer;
        return str;
    }

    // Encoding conversion.
    char cbuffer[1056];
    size_t inb = strlen(buffer);
    size_t outb = 1056;
    char* bp = cbuffer;
#ifdef __APPLE__
    char* sp = buffer;
#else
    const char* sp = buffer;
#endif
    iconv(m_cd, &sp, &inb, &bp, &outb);
    str = cbuffer;
#endif
    return str;
}

/**
 * Reads until either the end of the stream has been reached or a valid keyword has been found.
 *
 * @param in - The input stream
 * @param outIdentifier - Reference for returning the keyword if one has been found.
 *
 * @return Returns true if a valid identifier has been found and false if the end of the stream has been reached.
 */
static bool readUntilValidIdentifier(std::istream& in, Identifier& outIdentifier)
{
    // internal buffer for storing previously read bytes
    std::array<char, 16> buf;
    std::size_t numBufBytes = 0;

    // pointer onto the characters of the outgoing identifier
    auto outChars = outIdentifier.chrs;

    // lambda for removing the first byte of the buffer
    auto removeFirstByte = [&buf, &numBufBytes]()
    {
        assert(!buf.empty());

        std::copy(buf.begin() + 1, buf.end(), buf.begin());
        -- numBufBytes;
    };

    // fill buffer up to the given amount of bytes
    auto readBytesUntil = [&buf, &numBufBytes, &in](const std::size_t numBytes)
    {
        assert(numBytes <= 16);

        if(numBytes > numBufBytes)
        {
            in.read(buf.data() + numBufBytes, numBytes - numBufBytes);
            numBufBytes = numBytes;
        }
    };

    // read as long as we have not reached the end of the stream
    for(;in; removeFirstByte())
    {
        // ensure, that there are at least twelve bytes in the buffer
        readBytesUntil(12);

        // interpret the first 3 characters (12 bytes)
        auto readFirstThreeChars = [&buf, &outChars]() -> bool
        {
            auto ptr = buf.begin();

            for (std::size_t i = 0; i < 3; ++i, ptr += 4)
            {
                // the first three bytes of the current double word have to be zero
                if (ptr[0] != 0 || ptr[1] != 0 || ptr[2] != 0)
                    return false;

                // extract character
                outChars[i] = ptr[3];
            }

            return true;
        };

        if(!readFirstThreeChars())
            continue;

        // check if we got the end identifier
        if(std::equal(outChars, outChars + 3, "END"))
        {
            outChars[3] = 0;
            return true;
        }

        // check/read the next 4 bytes, i.e., the last character
        auto readLastChar = [&buf, &readBytesUntil, &outChars]() -> bool
        {
            auto ptr = buf.begin() + 12;

            // Check that the first 3 bytes are zero.
            // Here, we are a little bit more carefull to read as few bytes as needed
            for(std::size_t i=0; i<3; ++i)
            {
                // check if buffer is large enough
                readBytesUntil(13 + i);

                // stop reading if an invalid character is encountered
                if(ptr[i] != 0)
                    return false;
            }

            // finally, read last byte
            readBytesUntil(16);
            outChars[3] = ptr[3];

            return true;
        };

        if(!readLastChar())
            continue;

        // finally check if identifier is valid
        if(outIdentifier.isValid())
            return true;
    }

    return false;
}

static FacetGroup& readFacetGroup_(std::istream& is, FacetGroup& res)
{
    res.clear();
    res.resize(read_<unsigned int>(is));

    for (auto& p : res)
    {
        p.resize(read_<unsigned int>(is));
        for (auto& g : p)
        {
            g.resize(read_<unsigned int>(is));
            for (auto& v : g)
            {
                float x = read_<float>(is);
                float y = read_<float>(is);
                float z = read_<float>(is);
                v.first = Vector3F(x, y, z);
                x = read_<float>(is);
                y = read_<float>(is);
                z = read_<float>(is);
                v.second = Vector3F(x, y, z);
            }
        }
    }
    return res;
}


template<typename T, size_t size>
void readArray_(std::istream &in, T(&a)[size])
{
    for (size_t i = 0; i < size; ++i)
        read_<T>(in, a[i]);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Implementation of a skip function
/// Surprisingly, multiple calling get() multiple times seems to be faster than seekg.
/////////////////////////////////////////////////////////////////////////////////////////////////////////

template<size_t numInts>
inline void skip_(std::istream& in)
{
    in.seekg(sizeof(int) * numInts, in.cur);
}

template<>
inline void skip_<1>(std::istream& in)
{
    in.get();
    in.get();
    in.get();
    in.get();
}

template<>
inline void skip_<2>(std::istream& in)
{
    skip_<1>(in);
    skip_<1>(in);
}

template<>
inline void skip_<3>(std::istream& in)
{
    skip_<1>(in);
    skip_<1>(in);
    skip_<1>(in);
}

namespace {

    static string trim(const string& s)
    {
        size_t si = s.find_first_not_of(" \n\r\t");
        if (si == string::npos) {
            return "";
        }
        size_t ei = s.find_last_not_of(" \n\r\t");
        return s.substr(si, ei - si + 1);
    }

    static string latin_to_utf8(const string& latin) {
        string result;
        for(uint8_t c: latin) {
            if(c < 0x80) {
                result += c;
            } else {
                // Found non-ASCII character, assume ISO 8859-1
                result += (0xc0 | (c & 0xc0) >> 6);
                result += (0x80 | (c & 0x3f));
            }
        }
        return result;
    }

    static void scaleMatrix(std::array<float, 12>& matrix, float factor) {
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 3; j++) {
                matrix[i*3+j] *= factor;
            }
        }
    }

}

RVMParser::RVMParser(RVMReader& reader) :
    m_reader(reader),
    m_objectName(""),
    m_objectFound(0),
    m_forcedColor(-1),
    m_scale(1.),
    m_nbGroups(0),
    m_nbPyramids(0),
    m_nbBoxes(0),
    m_nbRectangularToruses(0),
    m_nbCircularToruses(0),
    m_nbEllipticalDishes(0),
    m_nbSphericalDishes(0),
    m_nbSnouts(0),
    m_nbCylinders(0),
    m_nbSpheres(0),
    m_nbLines(0),
    m_nbFacetGroups(0),
    m_attributeStream(0),
    m_attributes(0),
    m_aggregation(false) {
}

bool RVMParser::readFile(const string& filename, bool ignoreAttributes)
{
    m_lastError = "";

    // Open RVM file
    ifstream is(filename.data(), ios::binary);
    if (!is.is_open()) {
        m_lastError = "Could not open file";
        return false;
    }

    // Try to find ATT companion file
    m_attributeStream = 0;
    filebuf afb;
    if (!ignoreAttributes) {
        string attfilename = filename.substr(0, filename.find_last_of(".")) + ".att";
        if (afb.open(attfilename.data(), ios::in)) {
            cout << "Found attribute companion file: " << attfilename << endl;
            m_attributeStream = new istream(&afb);
        } else {
            attfilename = filename.substr(0, filename.find_last_of(".")) + ".ATT";
            if (afb.open(attfilename.data(), ios::in)) {
                cout << "Found attribute companion file: " << attfilename << endl;
                m_attributeStream = new istream(&afb);
            }
        }
        if (m_attributeStream && !m_attributeStream->eof()) {
            std::getline(*m_attributeStream, m_currentAttributeLine, '\n');
        }
    }


    bool success = readStream(is);

    is.close();

    return success;
}

bool RVMParser::readFiles(const vector<string>& filenames, const string& name, bool ignoreAttributes)
{
    bool success = true;

    m_reader.startDocument();
    m_reader.startHeader("PMUC - Plant Mock-Up Converter", "Aggregation file", "", "", "");
    m_reader.endHeader();
    m_reader.startModel(name, "Aggregation");

    m_aggregation = true;

    auto zeroTranslation = Vector3F();
    for (unsigned int i = 0; i < filenames.size(); i++)
    {
        string groupname = filenames[i].substr(filenames[i].rfind(PATHSEP) + 1, filenames[i].find_last_of("."));
        m_reader.startGroup(groupname, zeroTranslation, 0);
        success = readFile(filenames[i], ignoreAttributes);
        if (!success) {
            break;
        }
        m_reader.endGroup();
    }

    m_reader.endModel();
    m_reader.endDocument();

    return success;
}

bool RVMParser::readBuffer(const char* buffer) {
    m_lastError = "";

    stringbuf sb(buffer);
    istream is(&sb);
    bool success = readStream(is);

    return success;
}

bool RVMParser::readStream(istream& is)
{
    Identifier id;
    if(!readUntilValidIdentifier(is, id))
    {
        m_lastError = "Could not find HEAD identifier";
        return false;
    }

    if (id.empty())
    {
        m_lastError = "Incorrect file format while reading identifier.";
        return false;
    }

    if (id != "HEAD")
    {
        m_lastError = "File header not found.";
        return false;
    }

    if (!m_aggregation)
        m_reader.startDocument();

    skip_<2>(is); // Garbage ?

    unsigned int version = read_<unsigned int>(is);

    string banner, fileNote, date, user;
    read_(is, banner);
    read_(is, fileNote);
    read_(is, date);
    read_(is, user);

    if (version >= 2)
    {
        read_(is, m_encoding);

        if (m_encoding == "Unicode UTF-8")
            m_encoding = "UTF-8";
    }
    else
        m_encoding = "UTF-8";


    if (!m_aggregation) {
        m_reader.startHeader(banner, fileNote, date, user, m_encoding);
        m_reader.endHeader();
    }

    if (!readUntilValidIdentifier(is, id)) {
        m_lastError = "Incorrect file format while reading identifier.";
        return false;
    }

    if (id.empty()) {
        m_lastError = "Incorrect file format while reading identifier.";
        return false;
    }
    if (id != "MODL") {
        m_lastError = "Model not found.";
        return false;
    }

    skip_<2>(is); // Garbage ?
    version = read_<unsigned int>(is);

    string projectName, name;
    read_(is, projectName);
    read_(is, name);

    if (!m_aggregation)
        m_reader.startModel(projectName, name);

    while ((read_(is, id)) != "END")
    {
        if (id == "CNTB") {
            if (!readGroup(is)) {
                return false;
            }
        } else if (id == "PRIM") {
            if (!readPrimitive(is)) {
                return false;
            }
        } else if (id == "COLR") {
            if (!readColor(is)) {
                return false;
            }
        } else {
            m_lastError = "'" + id.toString() + "' Unknown or invalid identifier found.";
            return false;
        }
    }

    if (!m_aggregation) {
        m_reader.endModel();
        // Garbage data ??
        m_reader.endDocument();
    }

    return true;
}

const string RVMParser::lastError()
{
    return m_lastError;
}

bool RVMParser::readGroup(std::istream& is)
{
    skip_<2>(is); // Garbage ?
    const unsigned int version = read_<unsigned int>(is);

    string name;
    read_(is, name);

    Vector3F translation;
    readArray_(is, translation.m_values);
    translation *= 0.001f;

    const unsigned int materialId = read_<unsigned int>(is);

    if (m_objectName.empty() || m_objectFound || name == m_objectName) {
        m_objectFound++;
    }
    if (m_objectFound)
    {
        m_nbGroups++;
        m_reader.startGroup(name, translation, m_forcedColor != -1 ? m_forcedColor : materialId);
        // Attributes
        if (m_attributeStream && !m_attributeStream->eof()) {
            string p;
            while (((p = trim(m_currentAttributeLine)) != "NEW " + name) && (!m_attributeStream->eof())) {
                std::getline(*m_attributeStream, m_currentAttributeLine, '\n');
            }
            if (p == "NEW " + name ) {
                m_reader.startMetaData();
                size_t i;
                std::getline(*m_attributeStream, m_currentAttributeLine, '\n');
                p = trim(latin_to_utf8(m_currentAttributeLine));
                while ((!m_attributeStream->eof()) && ((i = p.find(":=")) != string::npos)) {
                     string an = p.substr(0, i);
                     string av = p.substr(i+4, string::npos);

                     m_reader.startMetaDataPair(an, av);
                     m_reader.endMetaDataPair();
                     m_attributes++;

                     std::getline(*m_attributeStream, m_currentAttributeLine, '\n');
                     p = trim(latin_to_utf8(m_currentAttributeLine));
                }
                m_reader.endMetaData();
            }
        }
    }

    // Children
    Identifier id;
    while ((read_(is, id)) != "CNTE") {
        if (id == "CNTB") {
            if (!readGroup(is)) {
                return false;
            }
        } else if (id == "PRIM") {
            if (!readPrimitive(is)) {
                return false;
            }
        } else {
            m_lastError = "Unknown or invalid identifier found.";
            return false;
        }
    }

    skip_<3>(is); // Garbage ?

    if (m_objectFound) {
        m_reader.endGroup();
        m_objectFound--;
    }

    return true;
}

bool RVMParser::readPrimitive(std::istream& is)
{
    skip_<2>(is); // Garbage ?
    const unsigned int version = read_<unsigned int>(is);
    const unsigned int primitiveKind = read_<unsigned int>(is);

    std::array<float, 12> matrix;
    readMatrix(is, matrix);
    scaleMatrix(matrix, m_scale);

    // skip bounding box
    skip_<6>(is);

    Primitive   primitive;
    FacetGroup  fc;
    if (m_objectFound)
    {
        switch (primitiveKind)
        {
            case 1:
                m_nbPyramids++;
                readArray_(is, primitive.pyramid.data);
                m_reader.createPyramid(matrix, primitive.pyramid);
            break;

            case 2:
                m_nbBoxes++;
                readArray_(is, primitive.box.len);
                m_reader.createBox(matrix, primitive.box);
             break;

            case 3:
                m_nbRectangularToruses++;
                readArray_(is, primitive.rTorus.data);
                m_reader.createRectangularTorus(matrix, primitive.rTorus);
            break;

            case 4:
                m_nbCircularToruses++;
                readArray_(is, primitive.cTorus.data);
                m_reader.createCircularTorus(matrix, primitive.cTorus);
            break;

            case 5:
                m_nbEllipticalDishes++;
                readArray_(is, primitive.eDish.data);
                m_reader.createEllipticalDish(matrix, primitive.eDish);
            break;

            case 6:
                m_nbSphericalDishes++;
                readArray_(is, primitive.sDish.data);
                m_reader.createSphericalDish(matrix, primitive.sDish);
            break;

            case 7:
                m_nbSnouts++;
                readArray_(is, primitive.snout.data);

                m_reader.createSnout(matrix, primitive.snout);
            break;

            case 8:
                m_nbCylinders++;
                readArray_(is, primitive.cylinder.data);
                m_reader.createCylinder(matrix, primitive.cylinder);
            break;

            case 9:
                m_nbSpheres++;
                read_(is, primitive.sphere);
                m_reader.createSphere(matrix, primitive.sphere);
            break;

            case 10: {
                m_nbLines++;
                float startx = read_<float>(is);
                float endx = read_<float>(is);
                m_reader.createLine(matrix, startx, endx);
            } break;

            case 11: {
                m_nbFacetGroups++;
                readFacetGroup_(is, fc);
                m_reader.createFacetGroup(matrix, fc);
            } break;

            default: {
                m_lastError = "Unknown primitive.";
                return false;
            }
        }
    } else {
        switch (primitiveKind) {
            case 1:
                skip_<7>(is);
            break;

            case 2:
                skip_<3>(is);
            break;

            case 3:
                skip_<4>(is);
            break;

            case 4:
                skip_<3>(is);
            break;

            case 5:
                skip_<2>(is);
            break;

            case 6:
                skip_<2>(is);
            break;

            case 7:
                skip_<9>(is);
            break;

            case 8:
                skip_<2>(is);
            break;

            case 9:
                skip_<1>(is);
            break;

            case 10:
                skip_<2>(is);
            break;

            case 11:
                readFacetGroup_(is, fc);
            break;

            default:
                m_lastError = "Unknown primitive.";
                return false;
        }
    }
    return true;
}


bool RVMParser::readColor(std::istream& is)
{
    const auto pos = int(is.tellg());

    skip_<2>(is); // Garbage ?
    const unsigned int version = read_<unsigned int>(is);
    const unsigned int index = read_<unsigned int>(is);

    std::array<std::uint8_t, 4> color;
    is.read(reinterpret_cast<char*>(color.data()), 4);

    m_reader.updateColorPalette(index, color);

    return true;
}

void RVMParser::readMatrix(istream& is, std::array<float, 12>& matrix)
{
    for (auto &value : matrix)
        value = read_<float>(is);
}
