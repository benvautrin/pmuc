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

#include "dummyreader.h"

#include <iostream>

using namespace std;

DummyReader::DummyReader()
    : RVMReader() {
}

DummyReader::~DummyReader() {
}

void DummyReader::startDocument() {
    cout << "startDocument" << endl;
}

void DummyReader::endDocument() {
    cout << "endDocument" << endl;
}

void DummyReader::startHeader(const string& banner, const string& fileNote, const string& date, const string& user, const string& encoding) {
    cout << "startHeader\n  " << banner << "\n  " << fileNote << "\n  " << date << "\n  " << user << "\n  " << encoding << endl;
}

void DummyReader::endHeader() {
    cout << "endHeader" << endl;
}

void DummyReader::startModel(const string& projectName, const string& name) {
    cout << "startModel" << endl;
}

void DummyReader::endModel() {
    cout << "endModel" << endl;
}

void DummyReader::startGroup(const std::string& name, const Vector3F& translation, const int& materialId) {
    cout << "startGroup\n  " << name << endl;
}

void DummyReader::endGroup() {
    cout << "endGroup" << endl;
}

void DummyReader::startMetaData() {
}

void DummyReader::endMetaData() {
}

void DummyReader::startMetaDataPair(const string &name, const string &value) {
}

void DummyReader::endMetaDataPair() {
}

void DummyReader::startPyramid(const vector<float>& matrix,
                          const float& xbottom,
                          const float& ybottom,
                          const float& xtop,
                          const float& ytop,
                          const float& height,
                          const float& xoffset,
                          const float& yoffset) {
    cout << "startPyramid" << endl;
}

void DummyReader::endPyramid() {
    cout << "endPyramid" << endl;
}

void DummyReader::startBox(const vector<float>& matrix,
                      const float& xlength,
                      const float& ylength,
                      const float& zlength) {
    cout << "startBox" << endl;
}

void DummyReader::endBox() {
    cout << "endBox" << endl;
}

void DummyReader::startRectangularTorus(const vector<float>& matrix,
                                   const float& rinside,
                                   const float& routside,
                                   const float& height,
                                   const float& angle) {
    cout << "startRectangularTorus" << endl;
}

void DummyReader::endRectangularTorus() {
    cout << "endRectangularTorus" << endl;
}

void DummyReader::startCircularTorus(const vector<float>& matrix,
                                const float& rinside,
                                const float& routside,
                                const float& angle) {
    cout << "startCircularTorus" << endl;
}

void DummyReader::endCircularTorus() {
    cout << "endCircularTorus" << endl;
}

void DummyReader::startEllipticalDish(const vector<float>& matrix,
                                 const float& diameter,
                                 const float& radius) {
    cout << "startEllipticalDish" << endl;
}

void DummyReader::endEllipticalDish() {
    cout << "endEllipticalDish" << endl;
}

void DummyReader::startSphericalDish(const vector<float>& matrix,
                                const float& diameter,
                                const float& height) {
    cout << "startSphericalDish" << endl;
}

void DummyReader::endSphericalDish() {
    cout << "endSphericalDish" << endl;
}

void DummyReader::startSnout(const vector<float>& matrix,
                        const float& dtop,
                        const float& dbottom,
                        const float& xoffset,
                        const float& yoffset,
                        const float& height,
                        const float& unknown1,
                        const float& unknown2,
                        const float& unknown3,
                        const float& unknown4) {
    cout << "startSnout" << endl;
}

void DummyReader::endSnout() {
    cout << "endSnout" << endl;
}

void DummyReader::startCylinder(const vector<float>& matrix,
                           const float& radius,
                           const float& height) {
    cout << "startCylinder" << endl;
}

void DummyReader::endCylinder() {
    cout << "endCylinder" << endl;
}

void DummyReader::startSphere(const vector<float>& matrix,
                         const float& diameter) {
    cout << "startSphere" << endl;
}

void DummyReader::endSphere() {
    cout << "endSphere" << endl;
}

void DummyReader::startLine(const vector<float>& matrix,
                       const float& startx,
                       const float& endx) {
    cout << "startLine" << endl;
}

void DummyReader::endLine() {
    cout << "endLine" << endl;
}

void DummyReader::startFacetGroup(const vector<float>& matrix,
                             const vector<vector<vector<pair<Vector3F, Vector3F> > > >& vertexes) {
    cout << "startFacetGroup" << endl;
}

void DummyReader::endFacetGroup() {
    cout << "endFacetGroup" << endl;
}
