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
    m_headerFound(false),
    m_objectName(""),
    m_objectFound(0),
    m_forcedColor(-1),
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
    m_cd(0),
    m_aggregation(false) {
}

bool RVMParser::readFile(const string& filename, bool ignoreAttributes) {
    m_lastError = "";

    // Open RVM file
    filebuf fb;
    if (!fb.open(filename.data(), ios::in)) {
        m_lastError = "Could not open file";
        return false;
    }
    istream is(&fb);

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

    fb.close();
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
    for (int i = 0; i < filenames.size(); i++) {
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
    m_cd = iconv_open("UTF-8", m_encoding.data());

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

    iconv_close(m_cd);

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
                char buffer[1056];
                size_t inb = m_currentAttributeLine.size();
                size_t outb = 1056;
                char* bp = buffer;
                char* sp = const_cast<char*>(m_currentAttributeLine.data());
                iconv(m_cd, &sp, &inb, &bp, &outb);
                m_currentAttributeLine = buffer;
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
                     char buffer[1056];
                     size_t inb = m_currentAttributeLine.size();
                     size_t outb = 1056;
                     char* bp = buffer;
                     char* sp = const_cast<char*>(m_currentAttributeLine.data());
                     iconv(m_cd, &sp, &inb, &bp, &outb);
                     m_currentAttributeLine = buffer;
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
                m_reader->startPyramid(matrix,
                                       readFloat(is),
                                       readFloat(is),
                                       readFloat(is),
                                       readFloat(is),
                                       readFloat(is),
                                       readFloat(is),
                                       readFloat(is));
                m_reader->endPyramid();
            } break;

            case 2: {
                m_nbBoxes++;
                m_reader->startBox(matrix,
                                   readFloat(is),
                                   readFloat(is),
                                   readFloat(is));
                m_reader->endBox();
            } break;

            case 3: {
                m_nbRectangularToruses++;
                m_reader->startRectangularTorus(matrix,
                                                readFloat(is),
                                                readFloat(is),
                                                readFloat(is),
                                                readFloat(is));
                m_reader->endRectangularTorus();
            } break;

            case 4: {
                m_nbCircularToruses++;
                m_reader->startCircularTorus(matrix,
                                             readFloat(is),
                                             readFloat(is),
                                             readFloat(is));
                m_reader->endCircularTorus();
            } break;

            case 5: {
                m_nbEllipticalDishes++;
                m_reader->startEllipticalDish(matrix,
                                              readFloat(is),
                                              readFloat(is));
                m_reader->endEllipticalDish();
            } break;

            case 6: {
                m_nbSphericalDishes++;
                m_reader->startSphericalDish(matrix,
                                             readFloat(is),
                                             readFloat(is));
                m_reader->endSphericalDish();
            } break;

            case 7: {
                m_nbSnouts++;
                m_reader->startSnout(matrix,
                                     readFloat(is),
                                     readFloat(is),
                                     readFloat(is),
                                     readFloat(is),
                                     readFloat(is),
                                     readFloat(is),
                                     readFloat(is),
                                     readFloat(is),
                                     readFloat(is));
                m_reader->endSnout();
            } break;

            case 8: {
                m_nbCylinders++;
                m_reader->startCylinder(matrix,
                                        readFloat(is),
                                        readFloat(is));
                m_reader->endCylinder();
            } break;

            case 9: {
                m_nbSpheres++;
                m_reader->startSphere(matrix,
                                      readFloat(is));
                m_reader->endSphere();
            } break;

            case 10: {
                m_nbLines++;
                m_reader->startLine(matrix,
                                    readFloat(is),
                                    readFloat(is));
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
    is.read(buffer, size);
    buffer[size] = 0;
    // Since I've never been able to find why there sometimes are strange characters at the end of some names, ignore them and truncate the string.
    for (unsigned int i = 0; i < size; i++) {
        if ((unsigned char)buffer[i] > 0x7E) {
            buffer[i] = 0;
            break;
        }
    }
    return buffer;

    /* Attempt to use encoding... Failed.
    if (!m_cd) {
        return buffer;
    }

    char cbuffer[1056];
    size_t inb = strlen(buffer);
    size_t outb = 1056;
    char* bp = cbuffer;
    char* sp = buffer;
    iconv(m_cd, &sp, &inb, &bp, &outb);
    return cbuffer;
    */
}

vector<float> RVMParser::readMatrix(istream& is) {
    vector<float> res;
    for (int i = 0; i < 12; i++) {
        // Why do we have to multiply by 1000. ?
        res.push_back(readFloat(is) * 1000.f);
    }
    return res;
}

vector<float> RVMParser::readBoundingBox(istream& is) {
    vector<float> res;
    for (int i = 0; i < 6; i++) {
        res.push_back(readFloat(is));
    }
    return res;
}

vector<float> RVMParser::readVector(istream& is) {
    vector<float> res;
    for (int i = 0; i < 3; i++) {
        res.push_back(readFloat(is));
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
                g.push_back(pair<Vector3F, Vector3F>(Vector3F(readFloat(is), readFloat(is), readFloat(is)), Vector3F(readFloat(is), readFloat(is), readFloat(is))));
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

