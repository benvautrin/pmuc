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

#include "stlconverter.h"

#include <iostream>
#include <set>

#include "../api/rvmcolorhelper.h"
#include "../api/rvmmeshhelper.h"
#include "../common/stringutils.h"

#define EIGEN_DONT_VECTORIZE
#define EIGEN_DISABLE_UNALIGNED_ARRAY_ASSERT

#include <Eigen/Geometry>

using namespace std;

STLConverter::STLConverter(const string& filename)
    : RVMReader(), mFile(filename.c_str(), fstream::out | fstream::binary), m_facetCount(0) {
}

STLConverter::~STLConverter() {
  if (mFile.is_open()) {
    mFile.close();
  };
}

void STLConverter::startDocument() {}

void STLConverter::endDocument() {
  cout << "Facets: " << m_facetCount << endl;
  // cout << "Bounding Box: " << endl;
  // cout << "Min: " << m_boundingBox.min() << "Max: " << m_boundingBox.max() << endl;

  // Go back to the place to write the facet count
  mFile.seekp(80);
  mFile.write((char*)&m_facetCount, 4);
}

void STLConverter::startHeader(const string& banner,
                               const string& fileNote,
                               const string& date,
                               const string& user,
                               const string& encoding) {
  // 80 bytes unsignificant header
  char header[80] = {0};
  mFile.write(header, 80);
  // 4 bytes number of facets
  // Write as a placeholder for now, fill the real number later
  mFile.write((char*)&m_facetCount, 4);
}

void STLConverter::endHeader() {}

void STLConverter::startModel(const string& projectName, const string& name) {}

void STLConverter::endModel() {}

void STLConverter::startGroup(const std::string& name, const Vector3F& translation, const int& materialId) {
  m_translations.push_back(translation);
}

void STLConverter::endGroup() {
  m_translations.pop_back();
}

void STLConverter::startMetaData() {}

void STLConverter::endMetaData() {}

void STLConverter::startMetaDataPair(const string& name, const string& value) {}

void STLConverter::endMetaDataPair() {}

void STLConverter::createPyramid(const std::array<float, 12>& matrix, const Primitives::Pyramid& pyramid) {
  writeMesh(matrix, RVMMeshHelper2::makePyramid(pyramid, m_maxSideSize, m_minSides), "RVMPyramid");
}

void STLConverter::createBox(const std::array<float, 12>& matrix, const Primitives::Box& box) {
  writeMesh(matrix, RVMMeshHelper2::makeBox(box, m_maxSideSize, m_minSides), "RVMBox");
}

void STLConverter::createRectangularTorus(const std::array<float, 12>& matrix,
                                          const Primitives::RectangularTorus& torus) {
  writeMesh(matrix, RVMMeshHelper2::makeRectangularTorus(torus, m_maxSideSize, m_minSides), "RVMRectangularTorus");
}

void STLConverter::createCircularTorus(const std::array<float, 12>& matrix, const Primitives::CircularTorus& torus) {
  auto sides = RVMMeshHelper2::infoCircularTorusNumSides(torus, m_maxSideSize, m_minSides);
  writeMesh(matrix, RVMMeshHelper2::makeCircularTorus(torus, sides.first, sides.second), "RVMCircularTorus");
}

void STLConverter::createEllipticalDish(const std::array<float, 12>& matrix, const Primitives::EllipticalDish& dish) {
  auto sideInfo = RVMMeshHelper2::infoEllipticalDishNumSides(dish, m_maxSideSize, m_minSides);
  writeMesh(matrix, RVMMeshHelper2::makeEllipticalDish(dish, sideInfo.first, sideInfo.second), "RVMEllipticalDish");
}

void STLConverter::createSphericalDish(const std::array<float, 12>& matrix, const Primitives::SphericalDish& dish) {
  writeMesh(matrix, RVMMeshHelper2::makeSphericalDish(dish, m_maxSideSize, m_minSides), "RVMSphericalDish");
}

void STLConverter::createSnout(const std::array<float, 12>& matrix, const Primitives::Snout& snout) {
  writeMesh(matrix,
            RVMMeshHelper2::makeSnout(snout, RVMMeshHelper2::infoSnoutNumSides(snout, m_maxSideSize, m_minSides)),
            "RVMSnout");
}

void STLConverter::createCylinder(const std::array<float, 12>& matrix, const Primitives::Cylinder& cylinder) {
  writeMesh(
      matrix,
      RVMMeshHelper2::makeCylinder(cylinder, RVMMeshHelper2::infoCylinderNumSides(cylinder, m_maxSideSize, m_minSides)),
      "RVMCylinder");
}

void STLConverter::createSphere(const std::array<float, 12>& matrix, const Primitives::Sphere& sphere) {
  writeMesh(matrix, RVMMeshHelper2::makeSphere(sphere, m_maxSideSize, m_minSides), "RVMSphere");
}

void STLConverter::createLine(const std::array<float, 12>& matrix, const float& thickness, const float& length) {}

void STLConverter::createFacetGroup(const std::array<float, 12>& matrix,
                                    const vector<vector<vector<Vertex> > >& vertexes) {
  Mesh meshData;

  RVMMeshHelper2::tesselateFacetGroup(vertexes, &meshData);

  writeMesh(matrix, meshData, "RVMFacetGroup");
}

Eigen::Vector3f STLConverter::calculateFaceNormal(const Eigen::Vector3f& p1,
                                                  const Eigen::Vector3f& p2,
                                                  const Eigen::Vector3f& p3) {
  Eigen::Vector3f u(p2 - p1);
  Eigen::Vector3f v(p3 - p1);
  return u.cross(v);
}

void STLConverter::writeMesh(const std::array<float, 12>& matrix, const Mesh& mesh, const std::string comment) {
  Eigen::Transform<float, 3, Eigen::Affine> t;
  t.setIdentity();
  for (unsigned int i = 0; i < 3; i++) {
    for (unsigned int j = 0; j < 4; j++) {
      t(i, j) = matrix[i + j * 3];
    }
  }

  for (unsigned int i = 0; i < mesh.positionIndex.size(); i += 3) {
    unsigned int idx1 = mesh.positionIndex.at(i);
    unsigned int idx2 = mesh.positionIndex.at(i + 1);
    unsigned int idx3 = mesh.positionIndex.at(i + 2);

    Eigen::Vector3f p1(mesh.positions.at(idx1).m_values);
    Eigen::Vector3f p2(mesh.positions.at(idx2).m_values);
    Eigen::Vector3f p3(mesh.positions.at(idx3).m_values);

    p1 = t * p1;
    p2 = t * p2;
    p3 = t * p3;

    m_boundingBox.extend(p1);
    m_boundingBox.extend(p2);
    m_boundingBox.extend(p3);

    // Face normal
    auto fn = calculateFaceNormal(p1, p2, p3);
    mFile.write((char*)&fn[0], 4);
    mFile.write((char*)&fn[1], 4);
    mFile.write((char*)&fn[2], 4);

    // Vertex 1
    mFile.write((char*)&p1[0], 4);
    mFile.write((char*)&p1[1], 4);
    mFile.write((char*)&p1[2], 4);

    // Vertex 2
    mFile.write((char*)&p2[0], 4);
    mFile.write((char*)&p2[1], 4);
    mFile.write((char*)&p2[2], 4);

    // Vertex 3
    mFile.write((char*)&p3[0], 4);
    mFile.write((char*)&p3[1], 4);
    mFile.write((char*)&p3[2], 4);

    // Attributes
    char attributes[2] = {0};
    mFile.write(attributes, 2);

    // Count the facets
    m_facetCount++;
  }

  if (!comment.empty()) {
    // Can STL carry comments?
  }
}
