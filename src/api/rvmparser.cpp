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

#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdio>

#include "rvmreader.h"

using namespace std;

#ifdef _WIN32
#define PATHSEP '\\'
#else
#define PATHSEP '/'
#endif

string trim(const string& s) {
    size_t si = s.find_first_not_of(" \n\r\t");
    if (si == string::npos) {
        return "";
    }
    size_t ei = s.find_last_not_of(" \n\r\t");
    return s.substr(si, ei - si + 1);
}

RVMParser::RVMParser(RVMReader* reader) :
    m_reader(reader),
    m_attributeStream(0),
    m_headerFound(false),
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

bool RVMParser::readFile(const string& filename, bool ignoreAttributes) {
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
    afb.close();

    return success;
}

bool RVMParser::readFiles(const vector<string>& filenames, const string& name, bool ignoreAttributes) {
    bool success = true;

    m_reader->startDocument();
    m_reader->startHeader("PMUC - Plant Mock-Up Converter", "Aggregation file", "", "", "");
    m_reader->endHeader();
    m_reader->startModel(name, "Aggregation");

    m_aggregation = true;
    for (unsigned int i = 0; i < filenames.size(); i++) {
        string groupname = filenames[i].substr(filenames[i].rfind(PATHSEP) + 1, filenames[i].find_last_of("."));
        m_reader->startGroup(groupname, vector<float>(3, 0), 0);
        success = readFile(filenames[i], ignoreAttributes);
        if (!success) {
            break;
        }
        m_reader->endGroup();
    }

    m_reader->endModel();
    m_reader->endDocument();

    return success;
}

bool RVMParser::readBuffer(const char* buffer) {
    m_lastError = "";

    stringbuf sb(buffer);
    istream is(&sb);
    bool success = readStream(is);

    return success;
}

bool RVMParser::readStream(istream& is) {

    string id;

    id = readIdentifier(is);
    if (id == "") {
        m_lastError = "Incorrect file format while reading identifier.";
        return false;
    }
    if (id != "HEAD") {
        m_lastError = "File header not found.";
        return false;
    }

    if (!m_aggregation) {
        m_reader->startDocument();
    }

    readInt(is); // Garbage ?
    readInt(is); // Garbage ?
    int version = readInt(is);
    string banner = readString(is);
    string fileNote = readString(is);
    string date = readString(is);
    string user = readString(is);

    m_encoding = version >= 2 ? readString(is) : "UTF-8";
	if (m_encoding == "Unicode UTF-8") {
		m_encoding = "UTF-8";
	}
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
        m_reader->startHeader(banner, fileNote, date, user, m_encoding);
        m_reader->endHeader();
    }

    id = readIdentifier(is);
    if (id == "") {
        m_lastError = "Incorrect file format while reading identifier.";
        return false;
    }
    if (id != "MODL") {
        m_lastError = "Model not found.";
        return false;
    }
    readInt(is); // Garbage ?
    readInt(is); // Garbage ?
    version = readInt(is);
    string projectName = readString(is);
    string name = readString(is);
    if (!m_aggregation) {
        m_reader->startModel(projectName, name);
    }

    while ((id = readIdentifier(is)) != "END") {
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
        m_reader->endModel();
        // Garbage data ??
        m_reader->endDocument();
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

const string RVMParser::lastError() {
    return m_lastError;
}

bool RVMParser::readGroup(std::istream& is) {
    readInt(is); // Garbage ?
    readInt(is); // Garbage ?
    int version = readInt(is);
    string name = readString(is);
    vector<float> translation = readVector(is);
    int materialId = readInt(is);

    if (m_objectName.empty() || m_objectFound || name == m_objectName) {
        m_objectFound++;
    }
    if (m_objectFound) {
        m_nbGroups++;

        m_reader->startGroup(name, translation, m_forcedColor != -1 ? m_forcedColor : materialId);

        // Attributes
        if (m_attributeStream && !m_attributeStream->eof()) {
            string p;
            while (((p = trim(m_currentAttributeLine)) != "NEW " + name) && (!m_attributeStream->eof())) {
                std::getline(*m_attributeStream, m_currentAttributeLine, '\n');
#ifdef ICONV_FOUND
                if (m_cd != (iconv_t)-1) {
                    char buffer[1056];
                    size_t inb = m_currentAttributeLine.size();
                    size_t outb = 1056;
                    char* bp = buffer;
#ifdef __APPLE__
                    char* sp = const_cast<char*>(m_currentAttributeLine.data());
#else
                    const char* sp = m_currentAttributeLine.data();
#endif
                    iconv(m_cd, &sp, &inb, &bp, &outb);
                    m_currentAttributeLine = buffer;
                }
#endif
            }
            if (p == "NEW " + name ) {
                m_reader->startMetaData();
                size_t i;
                std::getline(*m_attributeStream, m_currentAttributeLine, '\n');
                p = trim(m_currentAttributeLine);
                while ((!m_attributeStream->eof()) && ((i = p.find(":=")) != string::npos)) {
                     string an = p.substr(0, i);
                     string av = p.substr(i+4, string::npos);
                     m_reader->startMetaDataPair(an, av);
                     m_reader->endMetaDataPair();
                     m_attributes++;

                     std::getline(*m_attributeStream, m_currentAttributeLine, '\n');
#ifdef ICONV_FOUND
                     if (m_cdatt != (iconv_t)-1) {
#ifdef __APPLE__
                         char* sp = const_cast<char*>(m_currentAttributeLine.data());
#else
                         const char* sp = m_currentAttributeLine.data();
#endif
                         size_t inLength = m_currentAttributeLine.size();
                          /* Assign enough space for UTF-8. */
                         char* utf8 = (char*) calloc(inLength*2,1);
                         char* utf8start = utf8;
                         size_t utf8Length = inLength*2;

                         iconv(m_cdatt, &sp, &inLength, &utf8, &utf8Length);
                         m_currentAttributeLine = string(utf8start);
                         delete utf8start;
                     }
#endif
                     p = trim(m_currentAttributeLine);
                }
                m_reader->endMetaData();
            }
        }
    }

    // Children
    string id;
    while ((id = readIdentifier(is)) != "CNTE") {
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

    readInt(is); // Garbage ?
    readInt(is); // Garbage ?
    readInt(is); // Garbage ?

    if (m_objectFound) {
        m_reader->endGroup();
        m_objectFound--;
    }

    return true;
}

bool RVMParser::readPrimitive(std::istream& is) {
    readInt(is); // Garbage ?
    readInt(is); // Garbage ?
    int version = readInt(is);
    int primitiveKind = readInt(is);
    vector<float> matrix = readMatrix(is);
    vector<float> boundingBox = readBoundingBox(is);

    if (m_objectFound) {
        switch (primitiveKind) {
            case 1: {
                m_nbPyramids++;
                float xbottom = readFloat(is);
                float ybottom = readFloat(is);
                float xtop = readFloat(is);
                float ytop = readFloat(is);
                float height = readFloat(is);
                float xoffset = readFloat(is);
                float yoffset = readFloat(is);
                m_reader->startPyramid(matrix,
                                       xbottom,
                                       ybottom,
                                       xtop,
                                       ytop,
                                       height,
                                       xoffset,
                                       yoffset);
                m_reader->endPyramid();
            } break;

            case 2: {
                m_nbBoxes++;
                float xlength = readFloat(is);
                float ylength = readFloat(is);
                float zlength = readFloat(is);
                m_reader->startBox(matrix,
                                   xlength,
                                   ylength,
                                   zlength);
                m_reader->endBox();
            } break;

            case 3: {
                m_nbRectangularToruses++;
                float rinside = readFloat(is);
                float routside = readFloat(is);
                float height = readFloat(is);
                float angle = readFloat(is);
                m_reader->startRectangularTorus(matrix,
                                                rinside,
                                                routside,
                                                height,
                                                angle);
                m_reader->endRectangularTorus();
            } break;

            case 4: {
                m_nbCircularToruses++;
                float rinside = readFloat(is);
                float routside = readFloat(is);
                float angle = readFloat(is);
                m_reader->startCircularTorus(matrix,
                                             rinside,
                                             routside,
                                             angle);
                m_reader->endCircularTorus();
            } break;

            case 5: {
                m_nbEllipticalDishes++;
                float diameter = readFloat(is);
                float radius = readFloat(is);
                m_reader->startEllipticalDish(matrix,
                                              diameter,
                                              radius);
                m_reader->endEllipticalDish();
            } break;

            case 6: {
                m_nbSphericalDishes++;
                float diameter = readFloat(is);
                float height = readFloat(is);
                m_reader->startSphericalDish(matrix,
                                             diameter,
                                             height);
                m_reader->endSphericalDish();
            } break;

            case 7: {
                m_nbSnouts++;
                float dtop = readFloat(is);
                float dbottom = readFloat(is);
                float height = readFloat(is);
                float xoffset = readFloat(is);
                float yoffset = readFloat(is);
                float unknown1 = readFloat(is);
                float unknown2 = readFloat(is);
                float unknown3 = readFloat(is);
                float unknown4 = readFloat(is);
                m_reader->startSnout(matrix,
                                     dtop,
                                     dbottom,
                                     height,
                                     xoffset,
                                     yoffset,
                                     unknown1,
                                     unknown2,
                                     unknown3,
                                     unknown4);
                m_reader->endSnout();
            } break;

            case 8: {
                m_nbCylinders++;
                float radius = readFloat(is);
                float height = readFloat(is);
                m_reader->startCylinder(matrix,
                                        radius,
                                        height);
                m_reader->endCylinder();
            } break;

            case 9: {
                m_nbSpheres++;
                float diameter = readFloat(is);
                m_reader->startSphere(matrix,
                                      diameter);
                m_reader->endSphere();
            } break;

            case 10: {
                m_nbLines++;
                float startx = readFloat(is);
                float endx = readFloat(is);
                m_reader->startLine(matrix,
                                    startx,
                                    endx);
                m_reader->endLine();
            } break;

            case 11: {
                m_nbFacetGroups++;
                m_reader->startFacetGroup(matrix,
                                          readFacetGroup(is));
                m_reader->endFacetGroup();
            } break;

            default: {
                m_lastError = "Unknown primitive.";
                return false;
            }
        }
    } else {
        switch (primitiveKind) {
            case 1: {
                readFloat(is);
                readFloat(is);
                readFloat(is);
                readFloat(is);
                readFloat(is);
                readFloat(is);
                readFloat(is);
            } break;

            case 2: {
                readFloat(is);
                readFloat(is);
                readFloat(is);
            } break;

            case 3: {
                readFloat(is);
                readFloat(is);
                readFloat(is);
                readFloat(is);
            } break;

            case 4: {
                readFloat(is);
                readFloat(is);
                readFloat(is);
            } break;

            case 5: {
                readFloat(is);
                readFloat(is);
            } break;

            case 6: {
                readFloat(is);
                readFloat(is);
            } break;

            case 7: {
                readFloat(is);
                readFloat(is);
                readFloat(is);
                readFloat(is);
                readFloat(is);
                readFloat(is);
                readFloat(is);
                readFloat(is);
                readFloat(is);
            } break;

            case 8: {
                readFloat(is);
                readFloat(is);
            } break;

            case 9: {
                readFloat(is);
            } break;

            case 10: {
                readFloat(is);
                readFloat(is);
            } break;

            case 11: {
                readFacetGroup(is);
            } break;

            default: {
                m_lastError = "Unknown primitive.";
                return false;
            }
        }
    }
    return true;
}

string RVMParser::readIdentifier(istream& is)
{
    string res;
    for (int j = 0; j < 3; j++) {
        for (int i = 0; i < 3; i++) {
            if (readChar(is) != 0) {
                return "";
            }
        }
        res.push_back(readChar(is));
    }
    if (res == "END") {
        return res;
    }
    for (int i = 0; i < 3; i++) {
        if (readChar(is) != 0) {
            return "";
        }
    }
    res.push_back(readChar(is));

    return res;
}

string RVMParser::readString(istream& is)
{
    string res;
    unsigned int size = readInt(is) * 4;
    if (size == 0)
        return "";
    char buffer[1024];
	buffer[0] = 0;
    is.read(buffer, size);
    buffer[size] = 0;
    // Since I've never been able to find why there sometimes are strange characters at the end of some names, ignore them and truncate the string.
	/*
	if (m_encoding != "UTF-8" && m_encoding != "Unicode UTF-8") { 
	    for (unsigned int i = 0; i < size; i++) {
		    if ((unsigned char)buffer[i] > 0x7E) {
			    buffer[i] = 0;
				break;
		    }
	    }
	}*/
	
#ifndef ICONV_FOUND
    return buffer;
#else
	// If already in UTF-8, no change
	if (m_cd == (iconv_t)-1) {
        return buffer;
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
    return cbuffer;
#endif
}

vector<float> RVMParser::readMatrix(istream& is) {
    vector<float> res;
    for (int i = 0; i < 12; i++) {
        // Why do we have to multiply by 1000. ?
        res.push_back(readFloat(is) * 1000.f);
    }
    for (int i = 9; i < 12; i++) {
        res[i] *= m_scale;
    }
    return res;
}

vector<float> RVMParser::readBoundingBox(istream& is) {
    vector<float> res;
    for (int i = 0; i < 6; i++) {
        res.push_back(readFloat(is) * m_scale);
    }
    return res;
}

vector<float> RVMParser::readVector(istream& is) {
    vector<float> res;
    for (int i = 0; i < 3; i++) {
        res.push_back(readFloat(is) * m_scale);
    }
    return res;
}

std::vector<std::vector<std::vector<std::pair<Vector3F, Vector3F> > > > RVMParser::readFacetGroup(std::istream& is) {
    std::vector<std::vector<std::vector<std::pair<Vector3F, Vector3F> > > > res;
    unsigned int pc = readInt(is);
    for (unsigned int k = 0; k < pc; k++) {
        std::vector<std::vector<std::pair<Vector3F, Vector3F> > > p;
        unsigned int gc = readInt(is);
        for (unsigned int j = 0; j < gc; j++) {
            std::vector<std::pair<Vector3F, Vector3F> > g;
            unsigned int vc = readInt(is);
            for (unsigned int i = 0; i < vc; i++) {
                float x = readFloat(is) * m_scale;
                float y = readFloat(is) * m_scale;
                float z = readFloat(is) * m_scale;
                Vector3F c(x, y, z);
                x = readFloat(is) * m_scale;
                y = readFloat(is) * m_scale;
                z = readFloat(is) * m_scale;
                Vector3F n(x, y, z);
				pair<Vector3F, Vector3F> v(c, n);
                g.push_back(v);
            }
            p.push_back(g);
        }
        res.push_back(p);
    }
    return res;
}

unsigned int RVMParser::readInt(istream& is)
{
    unsigned int res;
    char* p  = (char*)(void*)&res;
    for (unsigned int i = 0; i < 4; i++) {
        p[3 - i] = readChar(is);
    }
    return res;
}

float RVMParser::readFloat(istream &is)
{
    float res;
    char* p  = (char*)(void*)&res;
    for (unsigned int i = 0; i < 4; i++) {
        p[3 - i] = readChar(is);
    }
    return res;
}

char RVMParser::readChar(istream& is)
{
    return is.get();
}

