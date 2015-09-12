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


#include "ifcconverter.h"


IFCConverter::IFCConverter(const std::string& filename) : RVMReader()
{
    
}

IFCConverter::~IFCConverter() {
}

void IFCConverter::startDocument() {
    // Done in startHeader
}

void IFCConverter::endDocument() {
    
}

void IFCConverter::startHeader(const std::string& banner, const std::string& fileNote, const std::string& date, const std::string& user, const std::string& encoding) {
    
}

void IFCConverter::endHeader() {
	
}

void IFCConverter::startModel(const std::string& projectName, const std::string& name) {
    
}

void IFCConverter::endModel() {
    
}

void IFCConverter::startGroup(const std::string& name, const std::vector<float>& translation, const int& materialId) {
    
}

void IFCConverter::endGroup() {
   
}

void IFCConverter::startMetaData() {
    
}

void IFCConverter::endMetaData() {
   
}

void IFCConverter::startMetaDataPair(const std::string &name, const std::string &value) {
   
}

void IFCConverter::endMetaDataPair() {
}

void IFCConverter::startPyramid(const std::vector<float>& matrix,
                          const float& xbottom,
                          const float& ybottom,
                          const float& xtop,
                          const float& ytop,
                          const float& height,
                          const float& xoffset,
                          const float& yoffset) {

    
}

void IFCConverter::endPyramid() {
    
}

void IFCConverter::startBox(const std::vector<float>& matrix,
                      const float& xlength,
                      const float& ylength,
                      const float& zlength) {
    
}

void IFCConverter::endBox() {
    
}

void IFCConverter::startRectangularTorus(const std::vector<float>& matrix,
                                   const float& rinside,
                                   const float& routside,
                                   const float& height,
                                   const float& angle) {
    
}

void IFCConverter::endRectangularTorus() {

}

void IFCConverter::startCircularTorus(const std::vector<float>& matrix,
                                const float& rinside,
                                const float& routside,
                                const float& angle) {

}

void IFCConverter::endCircularTorus() {

}

void IFCConverter::startEllipticalDish(const std::vector<float>& matrix,
                                 const float& diameter,
                                 const float& radius) {
}

void IFCConverter::endEllipticalDish() {
}

void IFCConverter::startSphericalDish(const std::vector<float>& matrix,
                                const float& diameter,
                                const float& height) {
   
}

void IFCConverter::endSphericalDish() {
    
}

void IFCConverter::startSnout(const std::vector<float>& matrix,
                        const float& dbottom,
                        const float& dtop,
                        const float& height,
                        const float& xoffset,
                        const float& yoffset,
                        const float& unknown1,
                        const float& unknown2,
                        const float& unknown3,
                        const float& unknown4) {
    
}

void IFCConverter::endSnout() {
}

void IFCConverter::startCylinder(const std::vector<float>& matrix,
                           const float& radius,
                           const float& height) {
}

void IFCConverter::endCylinder() {
}

void IFCConverter::startSphere(const std::vector<float>& matrix,
                         const float& diameter) {
}

void IFCConverter::endSphere() {
}

void IFCConverter::startLine(const std::vector<float>& matrix,
                       const float& startx,
                       const float& endx) {
}

void IFCConverter::endLine() {
}



void IFCConverter::startFacetGroup(const std::vector<float>& matrix, const FGroup& vertexes) {
}

void IFCConverter::endFacetGroup() {
}

