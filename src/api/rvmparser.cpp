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
#include <algorithm>
#include <array>

#include "rvmreader.h"

using namespace std;

#ifdef _WIN32
#define PATHSEP '\\'
#else
#define PATHSEP '/'
#endif

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
static Identifier& read_<Identifier>(std::istream& in, Identifier& res)
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
    for(unsigned int i=0;i<size;++i)
        str[i] = is.get();

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

static FacetGroup& readFacetGroup_(std::istream& is, FacetGroup& res, float scale)
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
                float x = read_<float>(is) * scale;
                float y = read_<float>(is) * scale;
                float z = read_<float>(is) * scale;
                v.first = Vector3F(x, y, z);
                x = read_<float>(is) * scale;
                y = read_<float>(is) * scale;
                z = read_<float>(is) * scale;
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

static string trim(const string& s)
{
    size_t si = s.find_first_not_of(" \n\r\t");
    if (si == string::npos) {
        return "";
    }
    size_t ei = s.find_last_not_of(" \n\r\t");
    return s.substr(si, ei - si + 1);
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
    m_attributes(0),
#ifdef ICONV_FOUND
    m_cd((iconv_t)-1),
    m_cdatt((iconv_t)-1),
#endif
    m_aggregation(false) {
}

bool RVMParser::readFile(const string& filename)
{
    m_lastError = "";

    // Open RVM file
    ifstream is(filename.data(), ios::binary);
    if (!is.is_open()) {
        m_lastError = "Could not open file";
        return false;
    }

    bool success = readStream(is);

    is.close();

    return success;
}

bool RVMParser::readFiles(const vector<string>& filenames, const string& name)
{
    bool success = true;

    m_reader.startDocument();
    m_reader.startHeader("PMUC - Plant Mock-Up Converter", "Aggregation file", "", "", "");
    m_reader.endHeader();
    m_reader.startModel(name, "Aggregation");

    m_aggregation = true;

    const float zeroTranslation[3] = { 0.0f, 0.0f, 0.0f };
    for (unsigned int i = 0; i < filenames.size(); i++)
    {
        string groupname = filenames[i].substr(filenames[i].rfind(PATHSEP) + 1, filenames[i].find_last_of("."));
        m_reader.startGroup(groupname, zeroTranslation, 0);
        success = readFile(filenames[i]);
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
    read_(is, id);

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

#ifdef ICONV_FOUND
    m_cd = (iconv_t)-1;
    if (m_encoding != "UTF-8" && m_encoding != "Unicode UTF-8") {
        m_cd = iconv_open("UTF-8", m_encoding.data());
        if (m_cd == (iconv_t)-1) {
            cout << "Unknown encoding: " << m_encoding << endl;
        }
    }
    // Attributes seem to be always "ISO_8859-1"
    m_cdatt = iconv_open("UTF-8", "ISO8859-1");
#endif

    if (!m_aggregation) {
        m_reader.startHeader(banner, fileNote, date, user, m_encoding);
        m_reader.endHeader();
    }

    read_(is, id);
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
        } else {
            m_lastError = "Unknown or invalid identifier found.";
            return false;
        }
    }

    if (!m_aggregation) {
        m_reader.endModel();
        // Garbage data ??
        m_reader.endDocument();
    }

#ifdef ICONV_FOUND
    if (m_cd != (iconv_t)-1) {
        iconv_close(m_cd);
    }
    if (m_cdatt != (iconv_t)-1) {
        iconv_close(m_cdatt);
    }
#endif

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

    float translation[3];
    readArray_(is, translation);
    const unsigned int materialId = read_<unsigned int>(is);

    if (m_objectName.empty() || m_objectFound || name == m_objectName) {
        m_objectFound++;
    }
    if (m_objectFound)
    {
        m_nbGroups++;
        m_reader.startGroup(name, translation, m_forcedColor != -1 ? m_forcedColor : materialId);
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
                skip_<4>(is);

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
                m_reader.startLine(matrix,
                                    startx,
                                    endx);
                m_reader.endLine();
            } break;

            case 11: {
                m_nbFacetGroups++;
                readFacetGroup_(is, fc, m_scale);
                m_reader.startFacetGroup(matrix, fc);
                m_reader.endFacetGroup();
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
                readFacetGroup_(is, fc, 1.0f);
            break;

            default:
                m_lastError = "Unknown primitive.";
                return false;
        }
    }
    return true;
}

void RVMParser::readMatrix(istream& is, std::array<float, 12>& matrix)
{
    for (auto &value : matrix)
        value = read_<float>(is) * 1000.f;

    for (size_t i = 9; i < 12; i++)
        matrix[i] *= m_scale;
}

