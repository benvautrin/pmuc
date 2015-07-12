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

#include "dslconverter.h"

#include <sstream>

#ifdef _MSC_VER
#define _USE_MATH_DEFINES // For PI under VC++
#endif

#include <math.h>

using namespace std;

DSLConverter::DSLConverter(const string& filename) :
    RVMReader(),
    m_lastShapeId(0)
{
    writer = new DSLWriter();
    writer->open(filename);
}

DSLConverter::~DSLConverter() {
    writer->close();
    delete writer;
}

void DSLConverter::startDocument() {
}

void DSLConverter::endDocument() {
}

void DSLConverter::startHeader(const string& banner, const string& fileNote, const string& date, const string& user, const string& encoding) {
}

void DSLConverter::endHeader() {
}

void DSLConverter::startModel(const string& projectName, const string& name) {
    m_groups.push_back(name);
    m_groupsChildren.push_back(vector<string>());
}

void DSLConverter::endModel() {
    writer->writeGroup(m_groups.back(), m_groupsChildren.back());
    // Export ?
}

void DSLConverter::startGroup(const std::string& name, const Vector3F& translation, const int& materialId) {
    string dslName = name;

    size_t p;
    while ((p = dslName.find_first_of(' ')) != string::npos)
        dslName[p] = '_';
    while ((p = dslName.find_first_of('-')) != string::npos)
        dslName[p] = '_';
    while ((p = dslName.find_first_of('/')) != string::npos) {
        dslName[p] = '_';
    }

    m_groups.push_back(dslName);
    m_groupsChildren.push_back(vector<string>());
    m_groupsTranslation.push_back(translation);
}

void DSLConverter::endGroup() {
    if (m_groupsChildren.back().size()) {
        writer->writeGroup(m_groups.back(), m_groupsChildren.back());
        m_groupsChildren.pop_back();
        m_groupsChildren.back().push_back(m_groups.back());
        m_groups.pop_back();
    } else {
        m_groupsChildren.pop_back();
        m_groups.pop_back();
    }
}

void DSLConverter::startMetaData() {
}

void DSLConverter::endMetaData() {
}

void DSLConverter::startMetaDataPair(const string &name, const string &value) {
}

void DSLConverter::endMetaDataPair() {
}

void DSLConverter::startPyramid(const vector<float>& matrix,
                          const float& xbottom,
                          const float& ybottom,
                          const float& xtop,
                          const float& ytop,
                          const float& height,
                          const float& xoffset,
                          const float& yoffset) {
    string shapeid = static_cast<ostringstream*>( &(ostringstream() << m_lastShapeId++) )->str();
    writer->writePyramid("bshape" + shapeid, xbottom, ybottom, xtop, ytop, height, xoffset, yoffset);
    writeShapeTransforms(shapeid, matrix);
}

void DSLConverter::endPyramid() {
}

void DSLConverter::startBox(const vector<float>& matrix,
                      const float& xlength,
                      const float& ylength,
                      const float& zlength) {
    string shapeid = static_cast<ostringstream*>( &(ostringstream() << m_lastShapeId++) )->str();
    writer->writeBox("bshape" + shapeid, xlength, ylength, zlength);
    writeShapeTransforms(shapeid, matrix);
}

void DSLConverter::endBox() {
}

void DSLConverter::startRectangularTorus(const vector<float>& matrix,
                                   const float& rinside,
                                   const float& routside,
                                   const float& height,
                                   const float& angle) {
    string shapeid = static_cast<ostringstream*>( &(ostringstream() << m_lastShapeId++) )->str();
    writer->writeRectangularTorus("bshape" + shapeid, routside, rinside, height, angle);
    writeShapeTransforms(shapeid, matrix);
}

void DSLConverter::endRectangularTorus() {
}

void DSLConverter::startCircularTorus(const vector<float>& matrix,
                                const float& rinside,
                                const float& routside,
                                const float& angle) {
    string shapeid = static_cast<ostringstream*>( &(ostringstream() << m_lastShapeId++) )->str();
    writer->writeCircularTorus("bshape" + shapeid, routside, rinside, angle);
    writeShapeTransforms(shapeid, matrix);
}

void DSLConverter::endCircularTorus() {
}

void DSLConverter::startEllipticalDish(const vector<float>& matrix,
                                 const float& diameter,
                                 const float& radius) {
    string shapeid = static_cast<ostringstream*>( &(ostringstream() << m_lastShapeId++) )->str();
    writer->writeDish("bshape" + shapeid, radius, diameter, radius);
    writeShapeTransforms(shapeid, matrix);
}

void DSLConverter::endEllipticalDish() {
}

void DSLConverter::startSphericalDish(const vector<float>& matrix,
                                const float& diameter,
                                const float& height) {
    string shapeid = static_cast<ostringstream*>( &(ostringstream() << m_lastShapeId++) )->str();
    writer->writeDish("bshape" + shapeid, height, diameter, diameter);
    writeShapeTransforms(shapeid, matrix);
}

void DSLConverter::endSphericalDish() {
}

void DSLConverter::startSnout(const vector<float>& matrix,
                        const float& dtop,
                        const float& dbottom,
                        const float& xoffset,
                        const float& yoffset,
                        const float& height,
                        const float& unknown1,
                        const float& unknown2,
                        const float& unknown3,
                        const float& unknown4) {
    string shapeid = static_cast<ostringstream*>( &(ostringstream() << m_lastShapeId++) )->str();
    writer->writeSnout("bshape" + shapeid, dbottom/2, dtop/2, xoffset, yoffset, height);
    writeShapeTransforms(shapeid, matrix);
}

void DSLConverter::endSnout() {
}

void DSLConverter::startCylinder(const vector<float>& matrix,
                           const float& radius,
                           const float& height) {
    string shapeid = static_cast<ostringstream*>( &(ostringstream() << m_lastShapeId++) )->str();
    writer->writeCylinder("bshape" + shapeid, radius, height);
    writeShapeTransforms(shapeid, matrix);
}

void DSLConverter::endCylinder() {
}

void DSLConverter::startSphere(const vector<float>& matrix,
                         const float& diameter) {
    string shapeid = static_cast<ostringstream*>( &(ostringstream() << m_lastShapeId++) )->str();
    writer->writeSphere("bshape" + shapeid, diameter/2);
    writeShapeTransforms(shapeid, matrix);
}

void DSLConverter::endSphere() {
}

void DSLConverter::startLine(const vector<float>& matrix,
                       const float& startx,
                       const float& endx) {
    string shapeid = static_cast<ostringstream*>( &(ostringstream() << m_lastShapeId++) )->str();
    writer->writeLine("bshape" + shapeid, startx, 0, 0, endx, 0, 0);
    writeShapeTransforms(shapeid, matrix);
}

void DSLConverter::endLine() {
}

void DSLConverter::startFacetGroup(const vector<float>& matrix,
                             const vector<vector<vector<pair<Vector3F, Vector3F> > > >& vertexes) {
    // Not supported.
}

void DSLConverter::endFacetGroup() {
}

void DSLConverter::writeShapeTransforms(const string& shapeid, const std::vector<float>& matrix) {
    float rx, ry, rz;
    if (matrix[4] > 0.998) {
        ry = atan2(matrix[2], matrix[10]);
        rz = (float)M_PI / 2;
        rx = 0;
    } else if (matrix[4] < -0.998) {
        ry = atan2(matrix[2], matrix[10]);
        rz = (float)-M_PI / 2;
        rx = 0;
    } else {
        ry = atan2(-matrix[8], matrix[0]);
        rz = asin(matrix[4]);
        rx = atan2(-matrix[6], matrix[5]);
    }

    writer->writeRotation("rshape" + shapeid, "bshape" + shapeid, rx, ry, rz);
    writer->writeTranslation("tshape" + shapeid, "rshape" + shapeid, matrix[3], matrix[7], matrix[11]);
    Vector3F gt = m_groupsTranslation.back();
    writer->writeTranslation("shape" + shapeid, "tshape" + shapeid, gt[0], gt[1], gt[2]);
    m_groupsChildren.back().push_back("shape" + shapeid);
}

