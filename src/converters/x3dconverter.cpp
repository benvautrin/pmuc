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

// Only compile if XIOT is available.

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
#include "../common/stringutils.h"

using namespace std;
using namespace XIOT;
using namespace Eigen;

X3DConverter::X3DConverter(const string& filename, bool binary) :
    RVMReader(),
    m_binary(binary),
    m_id(0)
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
    startNode(ID::WorldInfo);
    MFString info;
    info.push_back("info");
    m_writers.back()->setMFString(ID::info, info);
    startNode(ID::MetadataSet);
    if (!projectName.empty()) {
        writeMetaDataString("projectName", projectName, true);
    }
    if (!name.empty()) {
        writeMetaDataString("name", name, true);
    }
    endNode(ID::MetadataSet); // MetaDataSet
    endNode(ID::WorldInfo); // WorldInfo
    startNode(ID::Background);
    m_writers.back()->setSFColor(ID::skyColor, .9f, .9f, .9f);
    endNode(ID::Background);
}

void X3DConverter::endModel() {
    // Forgot something ?
}

void X3DConverter::startGroup(const std::string& name, const Vector3F& translation, const int& materialId) {
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
        startNode(ID::Inline);

        MFString name;
        name.push_back(x3dName + ".x3d");
        m_writers.back()->setMFString(ID::url, name);

        X3DWriter* writer = m_binary ? (X3DWriter*)new X3DWriterFI() : (X3DWriter*)new X3DWriterXML();
        writer->setProperty(Property::IntEncodingAlgorithm, (void*)Encoder::DeltazlibIntArrayEncoder);
        writer->setProperty(Property::FloatEncodingAlgorithm, (void*)Encoder::QuantizedzlibFloatArrayEncoder);
        writer->openFile((x3dName + ".x3d").data());
        m_writers.push_back(writer);
        multimap<std::string, std::string> meta;
        meta.insert(pair<string, string>("generator", "Plant Mock-Up Converter 0.1"));
        m_writers.back()->startX3DDocument(Immersive, VERSION_3_0, &meta, false);
        startNode(ID::Background);
        m_writers.back()->setSFColor(ID::skyColor, .9f, .9f, .9f);
        endNode(ID::Background);
    }
    startNode(ID::Transform);
	// Problems with encoding and unicity so changed name from DEF storage to metadata. See after translation.
    //m_writers.back()->setSFString(ID::DEF, x3dName);
    /*m_writers.back()->setSFVec3f(ID::translation,
                                 (translation[0] - m_translations.back()[0]),
                                 (translation[1] - m_translations.back()[1]),
                                 (translation[2] - m_translations.back()[2]));*/
    m_translations.push_back(translation);

    // PDMS name as metadata.
    // writeMetaDataString("pdms", name);
}

void X3DConverter::endGroup() {
    m_translations.pop_back();
    m_materials.pop_back();
    m_groups.pop_back();
    endNode(ID::Transform); // Transform
    if (m_split) {
        m_writers.back()->endX3DDocument();
        m_writers.back()->flush();
        m_writers.back()->closeFile();
        m_writers.pop_back();
        endNode(ID::Inline);
    }
}

void X3DConverter::startMetaData() {
    startNode(ID::MetadataSet);
    m_writers.back()->setSFString(ID::containerField, "metadata");
}

void X3DConverter::endMetaData() {
    endNode(ID::MetadataSet); // MetadataSet
}

void X3DConverter::startMetaDataPair(const string &name, const string &value) {
    writeMetaDataString(name, value, true);
}

void X3DConverter::endMetaDataPair() {
}

void X3DConverter::createPyramid(const std::array<float, 12>& matrix, const Primitives::Pyramid& pyramid) {

    startShape(matrix);

    std::vector<float> params;
    params.push_back(Pyramid);
    params.push_back(pyramid.xbottom());
    params.push_back(pyramid.ybottom());
    params.push_back(pyramid.xtop());
    params.push_back(pyramid.ytop());
    params.push_back(pyramid.height());
    params.push_back(pyramid.xoffset());
    params.push_back(pyramid.yoffset());

    pair<string,int> gid = getInstanceName(params);
    if(gid.first.empty()) {
        gid.first = createGeometryId();
        Mesh c = RVMMeshHelper2::makePyramid(pyramid, m_maxSideSize, m_minSides);
        gid.second = startMeshGeometry(c, gid.first);
        m_instanceMap.insert(std::make_pair(params, gid));
    } else {
        writeMeshInstance(gid.second, gid.first);
    }
    endNode(ID::IndexedTriangleSet);
    endShape();
}

void X3DConverter::createBox(const std::array<float, 12>& matrix, const Primitives::Box& box) {
    startShape(matrix);
    std::vector<float> params;
    params.push_back(Box);
    params.push_back(box.len[0]);
    params.push_back(box.len[1]);
    params.push_back(box.len[2]);

    if (m_primitives) {
        startNode(ID::Box);
        m_writers.back()->setSFVec3f(ID::size, box.len[0], box.len[1], box.len[2]);
        endNode(ID::Box);
    } else {
        pair<string,int> gid = getInstanceName(params);
        if(gid.first.empty()) {
            gid.first = createGeometryId();
            Mesh c = RVMMeshHelper2::makeBox(box, m_maxSideSize, m_minSides);
            gid.second = startMeshGeometry(c, gid.first);
            m_instanceMap.insert(std::make_pair(params, gid));
        } else {
            writeMeshInstance(gid.second, gid.first);
        }
        endNode(ID::IndexedTriangleSet);
    }
    endShape();
}


void X3DConverter::createRectangularTorus(const std::array<float, 12>& matrix, const Primitives::RectangularTorus& torus) {

    startShape(matrix);
    std::vector<float> params;
    params.push_back(RectangularTorus);
    params.push_back(torus.rinside());
    params.push_back(torus.routside());
    params.push_back(torus.height());
    params.push_back(torus.angle());

    pair<string,int> gid = getInstanceName(params);
    if(gid.first.empty()) {
        gid.first = createGeometryId();
        Mesh c = RVMMeshHelper2::makeRectangularTorus(torus, m_maxSideSize, m_minSides);
        gid.second = startMeshGeometry(c, gid.first);
        m_instanceMap.insert(std::make_pair(params, gid));
    } else {
        writeMeshInstance(gid.second, gid.first);
    }
    endNode(ID::IndexedFaceSet);
    endShape();

}

void X3DConverter::createCircularTorus(const std::array<float, 12>& matrix, const Primitives::CircularTorus& torus) {
    startShape(matrix);

    std::vector<float> params;
    params.push_back(CircularTorus);
    params.push_back(torus.offset());
    params.push_back(torus.radius());
    params.push_back(torus.angle());

    pair<string,int> gid = getInstanceName(params);
    if(gid.first.empty()) {
        gid.first = createGeometryId();
        auto sides = RVMMeshHelper2::infoCircularTorusNumSides(torus, m_maxSideSize, m_minSides);
        Mesh c = RVMMeshHelper2::makeCircularTorus(torus, sides.first, sides.second);
        gid.second = startMeshGeometry(c, gid.first);
        m_instanceMap.insert(std::make_pair(params, gid));
    } else {
        writeMeshInstance(gid.second, gid.first);
    }
    endNode(ID::IndexedFaceSet);
    endShape();
}

void X3DConverter::createEllipticalDish(const std::array<float, 12>& matrix, const Primitives::EllipticalDish& dish) {
    startShape(matrix);

    std::vector<float> params;
    params.push_back(EllipticalDish);
    params.push_back(dish.diameter());
    params.push_back(dish.radius());

    pair<string,int> gid = getInstanceName(params);
    if(gid.first.empty()) {
        gid.first = createGeometryId();
        auto sides = RVMMeshHelper2::infoEllipticalDishNumSides(dish, m_maxSideSize, m_minSides);
        Mesh c = RVMMeshHelper2::makeEllipticalDish(dish, sides.first, sides.second);
        
        gid.second = startMeshGeometry(c, gid.first);
        m_instanceMap.insert(std::make_pair(params, gid));
    } else {
        writeMeshInstance(gid.second, gid.first);
    }
    endNode(ID::IndexedTriangleSet);
    endShape();
}


void X3DConverter::createSphericalDish(const std::array<float, 12>& matrix, const Primitives::SphericalDish& dish) {
    startShape(matrix);

    std::vector<float> params;
    params.push_back(SphericalDish);
    params.push_back(dish.diameter());
    params.push_back(dish.height());

    pair<string,int> gid = getInstanceName(params);
    if(gid.first.empty()) {
        gid.first = createGeometryId();
        Mesh c = RVMMeshHelper2::makeSphericalDish(dish, m_maxSideSize, m_minSides);
        gid.second = startMeshGeometry(c, gid.first);
        m_instanceMap.insert(std::make_pair(params, gid));
    } else {
        writeMeshInstance(gid.second, gid.first);
    }
    endNode(ID::IndexedTriangleSet);
    endShape();
}


void X3DConverter::createSnout(const std::array<float, 12>& matrix, const Primitives::Snout& snout) {
    if (snout.height() == 0 && snout.dbottom() == 0 && snout.dtop() == 0) { // Degenerated snout...
        cerr << "Error: Found degenerated snout. Skipping data ..." << endl;
        return;
    }
    startShape(matrix);

    std::vector<float> params;
    params.push_back(Snout);
    params.push_back(snout.dtop());
    params.push_back(snout.dbottom());
    params.push_back(snout.height());
    params.push_back(snout.xoffset());
    params.push_back(snout.yoffset());
    params.push_back(snout.xbshear());
    params.push_back(snout.ybshear());
    params.push_back(snout.xtshear());
    params.push_back(snout.ytshear());

    pair<string,int> gid = getInstanceName(params);
    if(gid.first.empty()) {
        Mesh c = RVMMeshHelper2::makeSnout(snout, RVMMeshHelper2::infoSnoutNumSides(snout, m_maxSideSize, m_minSides));
        gid.first = createGeometryId();
        gid.second = startMeshGeometry(c, gid.first);
        m_instanceMap.insert(std::make_pair(params, gid));
    } else {
        writeMeshInstance(gid.second, gid.first);
    }
    endNode(ID::IndexedFaceSet);
    endShape();
}

void X3DConverter::createCylinder(const std::array<float, 12>& matrix, const Primitives::Cylinder& cylinder) {
    startShape(matrix);

    if (m_primitives) {
        startNode(ID::Cylinder);
        m_writers.back()->setSFFloat(ID::radius, cylinder.radius());
        m_writers.back()->setSFFloat(ID::height, cylinder.height());
        endNode(ID::Cylinder);
    } else {
        std::vector<float> params;
        params.push_back(Cylinder);
        params.push_back(cylinder.radius());
        params.push_back(cylinder.height());

        pair<string,int> gid = getInstanceName(params);
        if(gid.first.empty()) {
            gid.first = createGeometryId();
            Mesh c = RVMMeshHelper2::makeCylinder(cylinder, RVMMeshHelper2::infoCylinderNumSides(cylinder, m_maxSideSize, m_minSides));
            gid.second = startMeshGeometry(c, gid.first);
            m_instanceMap.insert(std::make_pair(params, gid));
        } else {
            writeMeshInstance(gid.second, gid.first);
        }
        endNode(ID::IndexedFaceSet);

    }
    endShape();

}

void X3DConverter::createSphere(const std::array<float, 12>& matrix, const Primitives::Sphere& sphere) {
    startShape(matrix);
    if (m_primitives) {
        startNode(ID::Sphere);
        m_writers.back()->setSFFloat(ID::radius, sphere.diameter);
        endNode(ID::Sphere);
    } else {
        std::vector<float> params;
        params.push_back(Sphere);
        params.push_back(sphere.diameter);

        pair<string,int> gid = getInstanceName(params);
        if(gid.first.empty()) {
            Mesh c = RVMMeshHelper2::makeSphere(sphere, m_maxSideSize, m_minSides);
            gid.first = createGeometryId();
            gid.second = startMeshGeometry(c, gid.first);
            m_instanceMap.insert(std::make_pair(params, gid));
        } else {
            writeMeshInstance(gid.second, gid.first);
        }
        endNode(ID::IndexedTriangleSet);
    }

    endShape();

}

void X3DConverter::createLine(const std::array<float, 12>& matrix,
                       const float& startx,
                       const float& endx) {
    startShape(matrix);
    startNode(ID::LineSet);
    m_writers.back()->setSFInt32(ID::vertexCount, 2);
    startNode(ID::Coordinate);
    vector<float> c;
    c.push_back(startx); c.push_back(0); c.push_back(0);
    c.push_back(endx); c.push_back(0); c.push_back(0);
    m_writers.back()->setMFFloat(ID::point, c);
    endNode(ID::Coordinate); // Coordinate
    endNode(ID::LineSet); // LineSet
    endShape();
}


int X3DConverter::startMeshGeometry(const Mesh &mesh, const string &id) {
    bool hasNormals = mesh.normals.size() > 0;
    bool hasNormalIndex = mesh.normalIndex.size() > 0;
    int meshType;

    // Not nice, but okay for now
    vector<int> index;
    vector<float> coordinates;
    for (unsigned int i = 0; i < mesh.positions.size(); i++) {
        for (unsigned int j = 0; j < 3; j++) {
            coordinates.push_back(mesh.positions.at(i)[j]);
        }
    }

    if(hasNormalIndex) { // We have to use a IndexedFaceSet
        meshType = ID::IndexedFaceSet;
        startNode(meshType);
        if(!id.empty()) {
            m_writers.back()->setSFString(ID::DEF, id);
        }
        m_writers.back()->setSFBool(ID::solid, false);

        vector<int> nindex;
        for (unsigned int i = 0; i < mesh.positionIndex.size(); i++) {
            index.push_back(mesh.positionIndex.at(i));
            if(i%3 == 2) {
                index.push_back(-1);
            }
        }
        m_writers.back()->setMFInt32(ID::coordIndex, index);

        for (unsigned int i = 0; i < mesh.normalIndex.size(); i++) {
            nindex.push_back(mesh.normalIndex.at(i));
            if(i%3 == 2) {
                nindex.push_back(-1);
            }
        }
        m_writers.back()->setMFInt32(ID::normalIndex, nindex);
    } else {
        meshType = ID::IndexedTriangleSet;
        startNode(meshType);
        if(!id.empty()) {
            m_writers.back()->setSFString(ID::DEF, id);
        }
        m_writers.back()->setSFBool(ID::solid, false);
        for (unsigned int i = 0; i < mesh.positionIndex.size(); i++) {
            index.push_back(mesh.positionIndex.at(i));
        }
        m_writers.back()->setMFInt32(ID::index, index);
    }

    startNode(ID::Coordinate);
    m_writers.back()->setMFFloat(ID::point, coordinates);
    endNode(ID::Coordinate);

    if(hasNormals) {
        vector<float> normals;
        for (unsigned int i = 0; i < mesh.normals.size(); i++) {
            for (unsigned int j = 0; j < 3; j++) {
                normals.push_back(mesh.normals.at(i)[j]);
            }
        }

        startNode(ID::Normal);
        m_writers.back()->setMFFloat(ID::vector, normals);
        endNode(ID::Normal);
    }
    return meshType;
}


void X3DConverter::createFacetGroup(const std::array<float, 12>& matrix,
                             const vector<vector<vector<pair<Vector3F, Vector3F> > > >& vertexes) {
    startShape(matrix);
    Mesh meshData;
    RVMMeshHelper2::tesselateFacetGroup(vertexes, &meshData);
    startMeshGeometry(meshData, "");
    endNode(ID::IndexedTriangleSet);
    endShape();
}

void X3DConverter::startShape(const std::array<float, 12>& matrix) {

    // Finding axis/angle from matrix using Eigen for its bullet proof implementation.
    Transform<float, 3, Affine> t;
    t.setIdentity();
    for (unsigned int i = 0; i < 3; i++) {
        for (unsigned int j = 0; j < 4; j++) {
            t(i, j) = matrix[i+j*3];
        }
    }

    Matrix3f rotationMatrix;
    Matrix3f scaleMatrix;
    t.computeRotationScaling(&rotationMatrix, &scaleMatrix);
	Quaternionf q;
    AngleAxisf aa;
	q = rotationMatrix;
    aa = q;

    Vector3f scale = scaleMatrix.diagonal();
    Vector3f translation = t.translation();

    startNode(ID::Transform);
    m_writers.back()->setSFVec3f(ID::translation, translation.x(), translation.y() , translation.z());
    m_writers.back()->setSFRotation(ID::rotation, aa.axis().x(), aa.axis().y(), aa.axis().z(), aa.angle());
    m_writers.back()->setSFVec3f(ID::scale, scale.x(), scale.y(), scale.z());
    startNode(ID::Shape);
    startNode(ID::Appearance);
    startNode(ID::Material);
    m_writers.back()->setSFColor<vector<float> >(ID::diffuseColor, RVMColorHelper::color(m_materials.back()));
    endNode(ID::Material); // Material
    endNode(ID::Appearance); // Appearance

}

void X3DConverter::endShape() {
    endNode(ID::Shape); // Shape
    endNode(ID::Transform); // Transform
}

void X3DConverter::writeMetaDataString(const string &name, const string &value, bool isValue) {
    startNode(ID::MetadataString);
    m_writers.back()->setSFString(ID::name, name);
    vector<string> v; v.push_back(escapeXMLAttribute(value));
    m_writers.back()->setMFString(ID::value, v);
    if(isValue)
        m_writers.back()->setSFString(ID::containerField, "value");
    endNode(ID::MetadataString);
}

void X3DConverter::writeMeshInstance(int meshType, const std::string &use) {
    // cerr << "Info: Writing instance of " << X3DTypes::getElementByID(meshType) << endl;
    startNode(meshType);
    m_writers.back()->setSFString(ID::USE, use);
}


void X3DConverter::startNode(int id) {
    m_nodeStack.push_back(id);
    // cerr << "Info: Starting node " << X3DTypes::getElementByID(id)  << endl;
    m_writers.back()->startNode(id);
}
void X3DConverter::endNode(int should) {
    if(should != -1) {
        int hasId = m_nodeStack.back();
        if(should != hasId) {
            cerr << "Error: Closing node " << X3DTypes::getElementByID(hasId) << ", but should be " << X3DTypes::getElementByID(should) << endl;
            exit(1);
        } else {
            // cerr << "Info: Closing node " << X3DTypes::getElementByID(hasId)  << endl;
        }
    }
    m_nodeStack.pop_back();

    m_writers.back()->endNode();
}

std::string X3DConverter::createGeometryId() {
        return "G" + toString(static_cast<long long>(m_id++));
}

std::pair<std::string, int> X3DConverter::getInstanceName(const std::vector<float> &params) {
    if(!m_split) {
        X3DInstanceMap::iterator I = m_instanceMap.find(params);
        if(I != m_instanceMap.end()) {
           return (*I).second;
        }
    }
    return std::make_pair("", 0);
}

