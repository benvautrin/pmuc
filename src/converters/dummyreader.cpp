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

void DummyReader::createPyramid(const std::array<float, 12>& matrix, const Primitives::Pyramid& params) {
    cout << "createPyramid" << endl;
}


void DummyReader::createBox(const std::array<float, 12>& matrix, const Primitives::Box& params) {
    cout << "createBox" << endl;
}


void DummyReader::createRectangularTorus(const std::array<float, 12>& matrix, const Primitives::RectangularTorus& params) {
    cout << "createRectangularTorus" << endl;
}


void DummyReader::createCircularTorus(const std::array<float, 12>& matrix, const Primitives::CircularTorus& params) {
    cout << "createCircularTorus" << endl;
}


void DummyReader::createEllipticalDish(const std::array<float, 12>& matrix, const Primitives::EllipticalDish& params) {
    cout << "startEllipticalDish" << endl;
}


void DummyReader::createSphericalDish(const std::array<float, 12>& matrix, const Primitives::SphericalDish& params) {
    cout << "createSphericalDish" << endl;
}


void DummyReader::createSnout(const std::array<float, 12>& matrix, const Primitives::Snout& params) {
    cout << "createSnout" << endl;
}


void DummyReader::createCylinder(const std::array<float, 12>& matrix, const Primitives::Cylinder& params) {
    cout << "createCylinder" << endl;
}


void DummyReader::createSphere(const std::array<float, 12>& matrix, const Primitives::Sphere& params) {
    cout << "createSphere" << endl;
}


void DummyReader::createLine(const std::array<float, 12>& matrix, const float& startx, const float& endx) {
    cout << "createLine" << endl;
}


void DummyReader::createFacetGroup(const std::array<float, 12>& matrix,  const vector<vector<vector<pair<Vector3F, Vector3F> > > >& vertexes) {
    cout << "createFacetGroup" << endl;
}
