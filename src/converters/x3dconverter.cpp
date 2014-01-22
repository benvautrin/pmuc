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

#include "x3dconverter.h"

#include <iostream>
#include <math.h>
#include <cmath>
#include <algorithm>

#include <xiot/X3DWriterFI.h>
#include <xiot/X3DWriterXML.h>

#include <Eigen/Dense>
#include <Eigen/Geometry>
#include <Eigen/SVD>

#include "../api/rvmcolorhelper.h"
#include "../api/rvmmeshhelper.h"
#include "../api/vector3f.h"

using namespace std;
using namespace XIOT;
using namespace Eigen;

// Helper function to escape XML attribute strings... Should have been done in XIOT...
string escapeXMLAttribute(const string& value) {
	string res = value;
    size_t pos = 0;
    while ((pos = res.find("&", pos + 3)) != string::npos) {
        res.replace(pos, 1, "&amp;");
    }
    while ((pos = res.find("<")) != string::npos) {
		res.replace(pos, 1, "&lt;");
	}
	while ((pos = res.find(">")) != string::npos) {
		res.replace(pos, 1, "&gt;");
    }
	while ((pos = res.find("\"")) != string::npos) {
		res.replace(pos, 1, "&quot;");
	}
	while ((pos = res.find("'")) != string::npos) {
		res.replace(pos, 1, "&apos;");
	}
	return res;
}


X3DConverter::X3DConverter(const string& filename, bool binary) :
    RVMReader(),
    m_binary(binary)
{
    X3DWriter* writer = binary ? (X3DWriter*)new X3DWriterFI() : (X3DWriter*)new X3DWriterXML();
    writer->setProperty(Property::IntEncodingAlgorithm, (void*)Encoder::DeltazlibIntArrayEncoder);
    writer->setProperty(Property::FloatEncodingAlgorithm, (void*)Encoder::QuantizedzlibFloatArrayEncoder);
    writer->openFile(filename.data());
    m_writers.push_back(writer);
}

X3DConverter::~X3DConverter() {
}

void X3DConverter::startDocument() {
    // Done in startHeader
}

void X3DConverter::endDocument() {
    m_writers.back()->endX3DDocument();
    m_writers.back()->flush();
    m_writers.back()->closeFile();
}

void X3DConverter::startHeader(const string& banner, const string& fileNote, const string& date, const string& user, const string& encoding) {
    multimap<std::string, std::string> meta;
    meta.insert(pair<string, string>("generator", "RVMConverter 0.1"));
    if (!banner.empty())
        meta.insert(pair<string, string>("rvmBanner", banner));
    if (!fileNote.empty())
        meta.insert(pair<string, string>("rvmFileNote", fileNote));
    if (!date.empty())
        meta.insert(pair<string, string>("rvmDate", date));
    if (!user.empty())
        meta.insert(pair<string, string>("rvmUser", user));
    m_writers.back()->startX3DDocument(Immersive, VERSION_3_0, &meta, false);

    vector<float> origin;
    origin.push_back(0.); origin.push_back(0.); origin.push_back(0.);
    m_translations.push_back(origin);
}

void X3DConverter::endHeader() {
	// Closed in startX3DDocument.
}

void X3DConverter::startModel(const string& projectName, const string& name) {
    m_writers.back()->startNode(ID::WorldInfo);
    MFString info;
    info.push_back("info");
    m_writers.back()->setMFString(ID::info, info);
    if (!projectName.empty()) {
        m_writers.back()->startNode(ID::MetadataString);
        m_writers.back()->setSFString(ID::name, "projectName");
        MFString pN;
        pN.push_back(projectName);
        m_writers.back()->setMFString(ID::value, pN);
        m_writers.back()->endNode();
    }
    if (!name.empty()) {
        m_writers.back()->startNode(ID::MetadataString);
        m_writers.back()->setSFString(ID::name, "name");
        MFString n;
        n.push_back(name);
        m_writers.back()->setMFString(ID::value, n);
        m_writers.back()->endNode();
    }
    m_writers.back()->endNode();
    m_writers.back()->startNode(ID::Background);
    m_writers.back()->setSFColor(ID::skyColor, .9f, .9f, .9f);
    m_writers.back()->endNode();
}

void X3DConverter::endModel() {
    // Forgot something ?
}

void X3DConverter::startGroup(const std::string& name, const std::vector<float>& translation, const int& materialId) {
    // Ensuring the name is compatible with the DEF attribute
    string x3dName = name;
    size_t p;
    while ((p = x3dName.find_first_of(' ')) != string::npos)
        x3dName[p] = '_';
    while (x3dName[0] == '-')
        x3dName[0] = '_';
    while ((p = x3dName.find_first_of('/')) != string::npos) {
        x3dName[p] = '_';
    }
    m_materials.push_back(materialId);
    m_groups.push_back(name);

    if (m_split) {
        m_writers.back()->startNode(ID::Inline);
        m_writers.back()->setSFString(ID::url, x3dName + ".x3d");

        X3DWriter* writer = m_binary ? (X3DWriter*)new X3DWriterFI() : (X3DWriter*)new X3DWriterXML();
        writer->setProperty(Property::IntEncodingAlgorithm, (void*)Encoder::DeltazlibIntArrayEncoder);
        writer->setProperty(Property::FloatEncodingAlgorithm, (void*)Encoder::QuantizedzlibFloatArrayEncoder);
        writer->openFile((x3dName + ".x3d").data());
        m_writers.push_back(writer);
        multimap<std::string, std::string> meta;
        meta.insert(pair<string, string>("generator", "Plant Mock-Up Converter 0.1"));
        m_writers.back()->startX3DDocument(Immersive, VERSION_3_0, &meta, false);
        m_writers.back()->startNode(ID::Background);
        m_writers.back()->setSFColor(ID::skyColor, .9f, .9f, .9f);
        m_writers.back()->endNode();
    }
    m_writers.back()->startNode(ID::Transform);
	// Problems with encoding and unicity so changed name from DEF storage to metadata. See after translation.
    //m_writers.back()->setSFString(ID::DEF, x3dName);
    m_writers.back()->setSFVec3f(ID::translation,
                                 (translation[0] - m_translations.back()[0]),
                                 (translation[1] - m_translations.back()[1]),
                                 (translation[2] - m_translations.back()[2]));
    m_translations.push_back(translation);

    // PDMS name as metadata.
    m_writers.back()->startNode(ID::MetadataString);
    m_writers.back()->setSFString(ID::name, "pdmsName");
    vector<string> v; v.push_back(escapeXMLAttribute(name));
    m_writers.back()->setMFString(ID::value, v);
    m_writers.back()->setSFString(ID::containerField, "metadata");
    m_writers.back()->endNode();
}

void X3DConverter::endGroup() {
    m_translations.pop_back();
    m_materials.pop_back();
    m_groups.pop_back();
    m_writers.back()->endNode(); // Transform
    if (m_split) {
        m_writers.back()->endX3DDocument();
        m_writers.back()->flush();
        m_writers.back()->closeFile();
        m_writers.pop_back();
        m_writers.back()->endNode();
    }
}

void X3DConverter::startMetaData() {
}

void X3DConverter::endMetaData() {
}

void X3DConverter::startMetaDataPair(const string &name, const string &value) {
    m_writers.back()->startNode(ID::MetadataString);
    m_writers.back()->setSFString(ID::name, name);
    vector<string> v; v.push_back(value);
    m_writers.back()->setMFString(ID::value, v);
    m_writers.back()->setSFString(ID::containerField, "metadata");
    m_writers.back()->endNode();
}

void X3DConverter::endMetaDataPair() {
}

void X3DConverter::startPyramid(const vector<float>& matrix,
                          const float& xbottom,
                          const float& ybottom,
                          const float& xtop,
                          const float& ytop,
                          const float& height,
                          const float& xoffset,
                          const float& yoffset) {
    startShape(matrix);
    pair<vector<vector<float> >, vector<vector<int> > > c = RVMMeshHelper::makePyramid(xbottom, ybottom, xtop, ytop, height, xoffset, yoffset, m_maxSideSize, m_minSides);
    vector<int> index;
    vector<float> coordinates;
    for (unsigned int i = 0; i < c.first.size(); i++) {
        for (unsigned int j = 0; j < c.first[i].size(); j++) {
            coordinates.push_back(c.first[i][j]);
        }
    }
    for (unsigned int i = 0; i < c.second.size(); i++) {
        for (unsigned int j = 0; j < c.second[i].size(); j++) {
            index.push_back(c.second[i][j]);
        }
    }
    if (index.size() != 0) { // Seems to happen with all parameters set to 0.
        m_writers.back()->startNode(ID::IndexedTriangleSet);
        m_writers.back()->setMFInt32(ID::index, index);
        m_writers.back()->startNode(ID::Coordinate);
        m_writers.back()->setMFFloat(ID::point, coordinates);
        m_writers.back()->endNode(); // Coordinate
        m_writers.back()->endNode(); // IndexedFaceSet
    }
}

void X3DConverter::endPyramid() {
    endShape();
}

void X3DConverter::startBox(const vector<float>& matrix,
                      const float& xlength,
                      const float& ylength,
                      const float& zlength) {
    startShape(matrix);
    if (m_primitives) {
        m_writers.back()->startNode(ID::Box);
        m_writers.back()->setSFVec3f(ID::size, xlength, ylength, zlength);
    } else {
        m_writers.back()->startNode(ID::IndexedTriangleSet);
        pair<vector<vector<float> >, vector<vector<int> > > c = RVMMeshHelper::makeBox(xlength, ylength, zlength, m_maxSideSize, m_minSides);
        vector<int> index;
        vector<float> coordinates;
        for (unsigned int i = 0; i < c.first.size(); i++) {
            for (unsigned int j = 0; j < 3; j++) {
                coordinates.push_back(c.first[i][j]);
            }
        }
        for (unsigned int i = 0; i < c.second.size(); i++) {
            for (unsigned int j = 0; j < c.second[i].size(); j++) {
                index.push_back(c.second[i][j]);
            }
        }
        m_writers.back()->setMFInt32(ID::index, index);
        m_writers.back()->startNode(ID::Coordinate);
        m_writers.back()->setMFFloat(ID::point, coordinates);
        m_writers.back()->endNode();
    }
}

void X3DConverter::endBox() {
    m_writers.back()->endNode();
    endShape();
}

void X3DConverter::startRectangularTorus(const vector<float>& matrix,
                                   const float& rinside,
                                   const float& routside,
                                   const float& height,
                                   const float& angle) {
    startShape(matrix);
    m_writers.back()->startNode(ID::IndexedTriangleSet);
    pair<pair<vector<vector<float> >, vector<vector<int> > >, pair<vector<vector<float> >, vector<vector<int> > > > c = RVMMeshHelper::makeRectangularTorus(rinside, routside, height, angle, m_maxSideSize, m_minSides);
    vector<int> index;
    vector<float> coordinates;
    for (unsigned int i = 0; i < c.first.first.size(); i++) {
        for (unsigned int j = 0; j < 3; j++) {
            coordinates.push_back(c.first.first[i][j]);
        }
    }
    for (unsigned int i = 0; i < c.first.second.size(); i++) {
        for (unsigned int j = 0; j < c.first.second[i].size(); j++) {
            index.push_back(c.first.second[i][j]);
        }
    }
    vector<int> normalindex;
    vector<float> normals;
    for (unsigned int i = 0; i < c.second.first.size(); i++) {
        for (unsigned int j = 0; j < 3; j++) {
            normals.push_back(c.second.first[i][j]);
        }
    }
    for (unsigned int i = 0; i < c.second.second.size(); i++) {
        for (unsigned int j = 0; j < c.second.second[i].size(); j++) {
            normalindex.push_back(c.second.second[i][j]);
        }
    }
    m_writers.back()->setMFInt32(ID::index, index);
    m_writers.back()->setMFInt32(ID::normalIndex, normalindex);
    m_writers.back()->startNode(ID::Coordinate);
    m_writers.back()->setMFFloat(ID::point, coordinates);
    m_writers.back()->endNode();
    m_writers.back()->startNode(ID::Normal);
    m_writers.back()->setMFFloat(ID::vector, normals);
    m_writers.back()->endNode();
}

void X3DConverter::endRectangularTorus() {
    m_writers.back()->endNode();
    endShape();
}

void X3DConverter::startCircularTorus(const vector<float>& matrix,
                                const float& rinside,
                                const float& routside,
                                const float& angle) {
    startShape(matrix);
    m_writers.back()->startNode(ID::IndexedTriangleSet);
    pair<pair<vector<vector<float> >, vector<vector<int> > >, pair<vector<vector<float> >, vector<vector<int> > > > c = RVMMeshHelper::makeCircularTorus(rinside, routside, angle, m_maxSideSize, m_minSides);
    vector<int> index;
    vector<float> coordinates;
    for (unsigned int i = 0; i < c.first.first.size(); i++) {
        for (unsigned int j = 0; j < 3; j++) {
            coordinates.push_back(c.first.first[i][j]);
        }
    }
    for (unsigned int i = 0; i < c.first.second.size(); i++) {
        for (unsigned int j = 0; j < c.first.second[i].size(); j++) {
            index.push_back(c.first.second[i][j]);
        }
    }
    vector<int> normalindex;
    vector<float> normals;
    for (unsigned int i = 0; i < c.second.first.size(); i++) {
        for (unsigned int j = 0; j < 3; j++) {
            normals.push_back(c.second.first[i][j]);
        }
    }
    for (unsigned int i = 0; i < c.second.second.size(); i++) {
        for (unsigned int j = 0; j < c.second.second[i].size(); j++) {
            normalindex.push_back(c.second.second[i][j]);
        }
    }
    m_writers.back()->setMFInt32(ID::index, index);
    m_writers.back()->setMFInt32(ID::normalIndex, normalindex);
    m_writers.back()->startNode(ID::Coordinate);
    m_writers.back()->setMFFloat(ID::point, coordinates);
    m_writers.back()->endNode();
    m_writers.back()->startNode(ID::Normal);
    m_writers.back()->setMFFloat(ID::vector, normals);
    m_writers.back()->endNode();
}

void X3DConverter::endCircularTorus() {
    m_writers.back()->endNode();
    endShape();
}

void X3DConverter::startEllipticalDish(const vector<float>& matrix,
                                 const float& diameter,
                                 const float& radius) {
    startShape(matrix);
    m_writers.back()->startNode(ID::IndexedTriangleSet);
    pair<pair<vector<vector<float> >, vector<vector<int> > >, pair<vector<vector<float> >, vector<vector<int> > > > c = RVMMeshHelper::makeEllipticalDish(diameter, radius, m_maxSideSize, m_minSides);
    vector<int> index;
    vector<float> coordinates;
    for (unsigned int i = 0; i < c.first.first.size(); i++) {
        for (unsigned int j = 0; j < 3; j++) {
            coordinates.push_back(c.first.first[i][j]);
        }
    }
    for (unsigned int i = 0; i < c.first.second.size(); i++) {
        for (unsigned int j = 0; j < c.first.second[i].size(); j++) {
            index.push_back(c.first.second[i][j]);
        }
    }
    vector<int> normalindex;
    vector<float> normals;
    for (unsigned int i = 0; i < c.second.first.size(); i++) {
        for (unsigned int j = 0; j < 3; j++) {
            normals.push_back(c.second.first[i][j]);
        }
    }
    for (unsigned int i = 0; i < c.second.second.size(); i++) {
        for (unsigned int j = 0; j < c.second.second[i].size(); j++) {
            normalindex.push_back(c.second.second[i][j]);
        }
    }
    m_writers.back()->setMFInt32(ID::index, index);
    m_writers.back()->setMFInt32(ID::normalIndex, normalindex);
    m_writers.back()->setSFBool(ID::solid, false);
    m_writers.back()->startNode(ID::Coordinate);
    m_writers.back()->setMFFloat(ID::point, coordinates);
    m_writers.back()->endNode();
    m_writers.back()->startNode(ID::Normal);
    m_writers.back()->setMFFloat(ID::vector, normals);
    m_writers.back()->endNode();
}

void X3DConverter::endEllipticalDish() {
    m_writers.back()->endNode();
    endShape();
}

void X3DConverter::startSphericalDish(const vector<float>& matrix,
                                const float& diameter,
                                const float& height) {
    startShape(matrix);
    m_writers.back()->startNode(ID::IndexedTriangleSet);
    pair<pair<vector<vector<float> >, vector<vector<int> > >, pair<vector<vector<float> >, vector<vector<int> > > > c = RVMMeshHelper::makeSphericalDish(diameter, height, m_maxSideSize, m_minSides);
    vector<int> index;
    vector<float> coordinates;
    for (unsigned int i = 0; i < c.first.first.size(); i++) {
        for (unsigned int j = 0; j < 3; j++) {
            coordinates.push_back(c.first.first[i][j]);
        }
    }
    for (unsigned int i = 0; i < c.first.second.size(); i++) {
        for (unsigned int j = 0; j < c.first.second[i].size(); j++) {
            index.push_back(c.first.second[i][j]);
        }
    }
    vector<int> normalindex;
    vector<float> normals;
    for (unsigned int i = 0; i < c.second.first.size(); i++) {
        for (unsigned int j = 0; j < 3; j++) {
            normals.push_back(c.second.first[i][j]);
        }
    }
    for (unsigned int i = 0; i < c.second.second.size(); i++) {
        for (unsigned int j = 0; j < c.second.second[i].size(); j++) {
            normalindex.push_back(c.second.second[i][j]);
        }
    }
    m_writers.back()->setMFInt32(ID::index, index);
    m_writers.back()->setMFInt32(ID::normalIndex, normalindex);
    m_writers.back()->setSFBool(ID::solid, false);
    m_writers.back()->startNode(ID::Coordinate);
    m_writers.back()->setMFFloat(ID::point, coordinates);
    m_writers.back()->endNode();
    m_writers.back()->startNode(ID::Normal);
    m_writers.back()->setMFFloat(ID::vector, normals);
    m_writers.back()->endNode();
}

void X3DConverter::endSphericalDish() {
    m_writers.back()->endNode();
    endShape();
}

void X3DConverter::startSnout(const vector<float>& matrix,
                        const float& dbottom,
                        const float& dtop,
                        const float& height,
                        const float& xoffset,
                        const float& yoffset,
                        const float& unknown1,
                        const float& unknown2,
                        const float& unknown3,
                        const float& unknown4) {
    if (height == 0 && dbottom == 0 && dtop == 0) { // Degenerated snout...
        return;
    }

    startShape(matrix);
    pair<pair<vector<vector<float> >, vector<vector<int> > >, pair<vector<vector<float> >, vector<vector<int> > > > c = RVMMeshHelper::makeSnout(dbottom, dtop, height, xoffset, yoffset, m_maxSideSize, m_minSides);
    vector<int> index;
    vector<float> coordinates;
    for (unsigned int i = 0; i < c.first.first.size(); i++) {
        for (unsigned int j = 0; j < 3; j++) {
            coordinates.push_back(c.first.first[i][j]);
        }
    }
    for (unsigned int i = 0; i < c.first.second.size(); i++) {
        for (unsigned int j = 0; j < c.first.second[i].size(); j++) {
            index.push_back(c.first.second[i][j]);
        }
    }
    vector<int> normalindex;
    vector<float> normals;
    for (unsigned int i = 0; i < c.second.first.size(); i++) {
        for (unsigned int j = 0; j < 3; j++) {
            normals.push_back(c.second.first[i][j]);
        }
    }
    for (unsigned int i = 0; i < c.second.second.size(); i++) {
        for (unsigned int j = 0; j < c.second.second[i].size(); j++) {
            normalindex.push_back(c.second.second[i][j]);
        }
    }
    m_writers.back()->startNode(ID::IndexedTriangleSet);
    m_writers.back()->setSFBool(ID::solid, false);
    m_writers.back()->setMFInt32(ID::index, index);
    m_writers.back()->setMFInt32(ID::normalIndex, normalindex);
    m_writers.back()->startNode(ID::Coordinate);
    m_writers.back()->setMFFloat(ID::point, coordinates);
    m_writers.back()->endNode(); // Coordinate
    m_writers.back()->startNode(ID::Normal);
    m_writers.back()->setMFFloat(ID::vector, normals);
    m_writers.back()->endNode(); // Normal
    m_writers.back()->endNode(); // IndexedFaceSet
    endShape();
}

void X3DConverter::endSnout() {
}

void X3DConverter::startCylinder(const vector<float>& matrix,
                           const float& radius,
                           const float& height) {
    startShape(matrix);
    if (m_primitives) {
        m_writers.back()->startNode(ID::Cylinder);
        m_writers.back()->setSFFloat(ID::radius, radius);
        m_writers.back()->setSFFloat(ID::height, height);
    } else {
        m_writers.back()->startNode(ID::IndexedTriangleSet);
        pair<pair<vector<vector<float> >, vector<vector<int> > >, pair<vector<vector<float> >, vector<vector<int> > > > c = RVMMeshHelper::makeCylinder(radius, height, m_maxSideSize, m_minSides);
        vector<int> index;
        vector<float> coordinates;
        for (unsigned int i = 0; i < c.first.first.size(); i++) {
            for (unsigned int j = 0; j < 3; j++) {
                coordinates.push_back(c.first.first[i][j]);
            }
        }
        for (unsigned int i = 0; i < c.first.second.size(); i++) {
            for (unsigned int j = 0; j < c.first.second[i].size(); j++) {
                index.push_back(c.first.second[i][j]);
            }
        }
        vector<int> normalindex;
        vector<float> normals;
        for (unsigned int i = 0; i < c.second.first.size(); i++) {
            for (unsigned int j = 0; j < 3; j++) {
                normals.push_back(c.second.first[i][j]);
            }
        }
        for (unsigned int i = 0; i < c.second.second.size(); i++) {
            for (unsigned int j = 0; j < c.second.second[i].size(); j++) {
                normalindex.push_back(c.second.second[i][j]);
            }
        }
        m_writers.back()->setMFInt32(ID::index, index);
        m_writers.back()->setMFInt32(ID::normalIndex, normalindex);
        m_writers.back()->startNode(ID::Coordinate);
        m_writers.back()->setMFFloat(ID::point, coordinates);
        m_writers.back()->endNode();
        m_writers.back()->startNode(ID::Normal);
        m_writers.back()->setMFFloat(ID::vector, normals);
        m_writers.back()->endNode();
    }
}

void X3DConverter::endCylinder() {
    m_writers.back()->endNode();
    endShape();
}

void X3DConverter::startSphere(const vector<float>& matrix,
                         const float& diameter) {
    startShape(matrix);
    if (m_primitives) {
        m_writers.back()->startNode(ID::Sphere);
        m_writers.back()->setSFFloat(ID::radius, diameter/2);
    } else {
        m_writers.back()->startNode(ID::IndexedTriangleSet);
        pair<pair<vector<vector<float> >, vector<vector<int> > >, pair<vector<vector<float> >, vector<vector<int> > > > c = RVMMeshHelper::makeSphere(diameter / 2, m_maxSideSize, m_minSides);
        vector<int> index;
        vector<float> coordinates;
        for (unsigned int i = 0; i < c.first.first.size(); i++) {
            for (unsigned int j = 0; j < 3; j++) {
                coordinates.push_back(c.first.first[i][j]);
            }
        }
        for (unsigned int i = 0; i < c.first.second.size(); i++) {
            for (unsigned int j = 0; j < c.first.second[i].size(); j++) {
                index.push_back(c.first.second[i][j]);
            }
        }
        vector<int> normalindex;
        vector<float> normals;
        for (unsigned int i = 0; i < c.second.first.size(); i++) {
            for (unsigned int j = 0; j < 3; j++) {
                normals.push_back(c.second.first[i][j]);
            }
        }
        for (unsigned int i = 0; i < c.second.second.size(); i++) {
            for (unsigned int j = 0; j < c.second.second[i].size(); j++) {
                normalindex.push_back(c.second.second[i][j]);
            }
        }
        m_writers.back()->setMFInt32(ID::index, index);
        m_writers.back()->setMFInt32(ID::normalIndex, normalindex);
        m_writers.back()->startNode(ID::Coordinate);
        m_writers.back()->setMFFloat(ID::point, coordinates);
        m_writers.back()->endNode();
        m_writers.back()->startNode(ID::Normal);
        m_writers.back()->setMFFloat(ID::vector, normals);
        m_writers.back()->endNode();
    }
}

void X3DConverter::endSphere() {
    m_writers.back()->endNode();
    endShape();
}

void X3DConverter::startLine(const vector<float>& matrix,
                       const float& startx,
                       const float& endx) {
    startShape(matrix);
    m_writers.back()->startNode(ID::LineSet);
    m_writers.back()->setSFInt32(ID::vertexCount, 2);
    m_writers.back()->startNode(ID::Coordinate);
    vector<float> c;
    c.push_back(startx); c.push_back(0); c.push_back(0);
    c.push_back(endx); c.push_back(0); c.push_back(0);
    m_writers.back()->setMFFloat(ID::point, c);
    m_writers.back()->endNode();
}

void X3DConverter::endLine() {
    m_writers.back()->endNode();
    endShape();
}

// From http://paulbourke.net/geometry/pointlineplane/lineline.c
bool intersect(const Vector3F& p1, const Vector3F& p2, const Vector3F& p3, const Vector3F& p4) {
    if (p1.equals(p2) || p1.equals(p3) || p1.equals(p4) || p2.equals(p3) || p2.equals(p4) || p3.equals(p4)) {
        return false;
    }
    Vector3F p13 = p1 - p3;
    Vector3F p43 = p4 - p3;
    if (abs(p43[0]) < .001 && abs(p43[1]) < .001 && abs(p43[2]) < .001)
        return false;
    Vector3F p21 = p2 - p1;
    if (abs(p21[0]) < .001 && abs(p21[1]) < .001 && abs(p21[2]) < .001)
        return false;

    float d1343 = p13 * p43;
    float d4321 = p43 * p21;
    float d1321 = p13 * p21;
    float d4343 = p43 * p43;
    float d2121 = p21 * p21;

    double denom = d2121 * d4343 - d4321 * d4321;
    if (abs(denom) < 0.001) {
        return false;
    }
    double numer = d1343 * d4321 - d1321 * d4343;

    float x1 = float(numer / denom);
    float x2 = (d1343 + d4321 * x1) / d4343;
    if (x1 > 1 || x1 < 0 || x2 > 1 || x2 < 0)
        return false;

    Vector3F pa = p1 + p21 * x1;
    Vector3F pb = p3 + p43 * x2;

    Vector3F papb = pb - pa;
    float n2 = papb * papb;
    if (n2 > 0.001)
        return false;

    return true;
}

void X3DConverter::startFacetGroup(const vector<float>& matrix,
                             const vector<vector<vector<pair<Vector3F, Vector3F> > > >& vertexes) {
    startShape(matrix);
    m_writers.back()->startNode(ID::IndexedFaceSet);

    vector<int> indexes;
    vector<float> coordinates;
    vector<int> normalindexes;
    vector<float> normals;

    for (unsigned int i = 0; i < vertexes.size(); i++) { // Patches
        if (vertexes[i].size() == 1) {
            int j = 0;
            if (vertexes[i][0].size() < 3) {
                continue;
            }
            for (unsigned int k = 0; k < vertexes[i][j].size(); k++) { // Vertexes
                int ci = -1;
                int ni = -1;
                vector<float> c(3, 0); c[0] = vertexes[i][j][k].first[0]; c[1] = vertexes[i][j][k].first[1]; c[2] = vertexes[i][j][k].first[2];
                vector<float> n(3, 0); n[0] = vertexes[i][j][k].second[0]; n[1] = vertexes[i][j][k].second[1]; n[2] = vertexes[i][j][k].second[2];
                // Try to find a corresponding point or insert the new one.
                for (unsigned int l = 0; l < coordinates.size(); l += 3) {
                    if (c[0] == coordinates[l] && c[1] == coordinates[l+1] && c[2] == coordinates[l+2]) {
                        ci = l/3;
                        continue;
                    }
                }
                if (ci == -1) {
                    for (unsigned int l = 0; l < 3; l++) {
                        coordinates.push_back(c[l]);
                    }
                    indexes.push_back(coordinates.size()/3-1);
                } else {
                    indexes.push_back(ci);
                }
                // Try to find a corresponding vector or insert the new one.
                for (unsigned int l = 0; l < normals.size(); l += 3) {
                    if (n[0] == normals[l] && n[1] == normals[l+1] && n[2] == normals[l+2]) {
                        ni = l/3;
                        continue;
                    }
                }
                if (ni == -1) {
                    for (unsigned int l = 0; l < 3; l++) {
                        normals.push_back(n[l]);
                    }
                    normalindexes.push_back(normals.size()/3-1);
                } else {
                    normalindexes.push_back(ni);
                }
            }
        } else {
            vector<pair<Vector3F, Vector3F> > polygon(vertexes[i][0]);
            vector<vector<pair<Vector3F, Vector3F> > > shapes;
            int reorder = 0;
            // Close shapes
            polygon.push_back(polygon[0]);
            for (unsigned int j = 1; j < vertexes[i].size(); j++) {
                shapes.push_back(vertexes[i][j]);
            }
            // Find non-crossing links and insert sub shapes
            while (!shapes.empty()) {
                vector<pair<Vector3F, Vector3F> > shape(shapes.back());
                bool noncrossing = true;
                int pi, si;
                for (unsigned int k = 0; k < (polygon.size()-1) * (shape.size()); k++) {
                    noncrossing = true;
                    // Choose one segment between the polygon and the shape
                    pi = k % (polygon.size()-1);
                    si = k / (polygon.size()-1);
                    // And test for collision with polygon
                    for (unsigned int l = 0; l < polygon.size()-1; l++) {
                        // Double check, probably for computing errors... How can we fix that ?
                        if (intersect(polygon[pi].first, shape[si].first, polygon[l].first, polygon[l+1].first) ||
                                intersect(shape[si].first, polygon[pi].first, polygon[l].first, polygon[l+1].first)) {
                            noncrossing = false;
                            continue;
                        }
                    }
                    // And with shapes
                    if (noncrossing && shapes.size()) {
                        for (unsigned int l = 0; l < shapes.size(); l++) {
                            for (unsigned int m = 0; m < shapes[l].size(); m++) {
                                if (intersect(polygon[pi].first, shape[si].first, shapes[l][m].first, shapes[l][m == shapes[l].size() - 1 ? 0 : m+1].first) ||
                                        intersect(shape[si].first, polygon[pi].first, shapes[l][m].first, shapes[l][m == shapes[l].size() - 1 ? 0 : m+1].first)) {
                                    noncrossing = false;
                                    continue;
                                }
                            }
                        }
                    }
                    if (noncrossing) {
                        break;
                    }
                }
                if (!noncrossing) {
                    // Very uncommon... Added a safe guard for infinite loops. 10 is totally arbitrary. Never met a face with more than one reordering.
                    reorder++;
                    if (reorder > 10) {
                        cout << "Could not find the decomposition of a face set. " << m_groups.back() << endl;
                        cout << "Ignoring one shape !!!!" << endl;
                    } else {
                        // Reordering shapes
                        shapes.insert(shapes.begin(), shape);
                    }
                } else {
                    // insert link and shape
					/* Problem here with VC2010. What causes inserts to change values ??
                    bool ins = false;
                    if (pi == 0 || !(polygon[pi-1].first.equals(polygon[pi].first)) || !(polygon[pi-1].second.equals(polygon[pi].second))) {
                        polygon.insert(polygon.begin() + pi, polygon[pi]);
                        ins = true;
                    }
                    for (unsigned int k = 0; k < shape.size() + 1; k++) {
                        polygon.insert(polygon.begin() + (pi + k + (ins ? 1 : 0)), shape[(k + si) % shape.size()]);
                    }
					*/
					/* New implementation compatible with VC2010 */
					vector<pair<Vector3F, Vector3F> > newpolygon;
					for (int k = 0; k < pi + 1; k++) {
						newpolygon.push_back(polygon[k]);
					}
                    for (unsigned int k = 0; k < shape.size() + 1; k++) {
                        newpolygon.push_back(shape[(k + si) % shape.size()]);
                    }
					for (unsigned int k = pi; k < polygon.size(); k++) {
						newpolygon.push_back(polygon[k]);
					}
					polygon.swap(newpolygon);
                }
                shapes.pop_back();
            }
            // Check polygon -- For debugging purposes.
            /*
            for (unsigned int k = 0; k < polygon.size() - 1; k++) {
                for (unsigned int l = k; l < polygon.size(); l++) {
                    if (intersect(polygon[k].first, polygon[k+1].first, polygon[l].first, polygon[l == polygon.size()-1 ? 0 : l+1].first)) {
                        // Should never show !
                        cout << "Collision on " << m_groups.back() << ": " << k << " " << l << endl;
                    }
                }
            }
            */
            // Then, insert polygon.
            if (polygon.size() < 3) {
                continue;
            }
            for (unsigned int k = 0; k < polygon.size(); k++) { // Vertexes
                int ci = -1;
                int ni = -1;
                vector<float> c(3, 0); c[0] = polygon[k].first[0]; c[1] = polygon[k].first[1]; c[2] = polygon[k].first[2];
                vector<float> n(3, 0); n[0] = polygon[k].second[0]; n[1] = polygon[k].second[1]; n[2] = polygon[k].second[2];
                for (unsigned int l = 0; l < coordinates.size(); l += 3) {
                    if (c[0] == coordinates[l] && c[1] == coordinates[l+1] && c[2] == coordinates[l+2]) {
                        ci = l/3;
                        continue;
                    }
                }
                if (ci == -1) {
                    for (unsigned int l = 0; l < 3; l++) {
                        coordinates.push_back(c[l]);
                    }
                    indexes.push_back(coordinates.size()/3-1);
                } else {
                    indexes.push_back(ci);
                }
                for (unsigned int l = 0; l < normals.size(); l += 3) {
                    if (n[0] == normals[l] && n[1] == normals[l+1] && n[2] == normals[l+2]) {
                        ni = l/3;
                        continue;
                    }
                }
                if (ni == -1) {
                    for (unsigned int l = 0; l < 3; l++) {
                        normals.push_back(n[l]);
                    }
                    normalindexes.push_back(normals.size()/3-1);
                } else {
                    normalindexes.push_back(ni);
                }
            }
        }
        indexes.push_back(-1);
        normalindexes.push_back(-1);
    }

    m_writers.back()->setSFBool(ID::solid, false);
    m_writers.back()->setSFBool(ID::convex, false);
    m_writers.back()->setSFFloat(ID::creaseAngle, 1.1f);
    m_writers.back()->setMFInt32(ID::coordIndex, indexes);
    m_writers.back()->setMFInt32(ID::normalIndex, normalindexes);
    m_writers.back()->startNode(ID::Coordinate);
    m_writers.back()->setMFFloat(ID::point, coordinates);
    m_writers.back()->endNode();
    m_writers.back()->startNode(ID::Normal);
    m_writers.back()->setMFFloat(ID::vector, normals);
    m_writers.back()->endNode();
}

void X3DConverter::endFacetGroup() {
    m_writers.back()->endNode(); // FaceSet
    endShape();
}

void X3DConverter::startShape(const std::vector<float>& matrix) {
    vector<float> r(4, 0);

    // Finding axis/angle from matrix using Eigen for its bullet proof implementation.
    Transform<double, 3, Affine> t;
    t.setIdentity();
    for (unsigned int i = 0; i < 3; i++) {
        for (unsigned int j = 0; j < 4; j++) {
            t(i, j) = matrix[i+j*3];
        }
    }
    Matrix3d rotation;
    Matrix3d scale;
    t.computeRotationScaling(&rotation, &scale);
	Quaterniond q;
    AngleAxisd aa;
	q = rotation;
    aa = q;
    r[0] = (float)aa.axis()(0);
    r[1] = (float)aa.axis()(1);
    r[2] = (float)aa.axis()(2);
    r[3] = (float)aa.angle();

    m_writers.back()->startNode(ID::Transform);
    m_writers.back()->setSFVec3f(ID::translation,
                                 (matrix[9] - m_translations.back()[0]),
                                 (matrix[10] - m_translations.back()[1]),
                                 (matrix[11] - m_translations.back()[2]));
    m_writers.back()->setMFRotation(ID::rotation, r);
    m_writers.back()->startNode(ID::Shape);
    m_writers.back()->startNode(ID::Appearance);
    m_writers.back()->startNode(ID::Material);
    m_writers.back()->setSFColor<vector<float> >(ID::diffuseColor, RVMColorHelper::color(m_materials.back()));
    m_writers.back()->endNode(); // Material
    m_writers.back()->endNode(); // Appearance

}

void X3DConverter::endShape() {
    m_writers.back()->endNode(); // Shape
    m_writers.back()->endNode(); // Transform
}
