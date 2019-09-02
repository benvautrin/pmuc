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

DSLConverter::DSLConverter(const string &filename) : RVMReader(),
                                                     m_lastShapeId(0)
{
    writer = new DSLWriter();
    writer->open(filename);
}

DSLConverter::~DSLConverter()
{
    writer->close();
    delete writer;
}

void DSLConverter::startDocument()
{
}

void DSLConverter::endDocument()
{
}

void DSLConverter::startHeader(const string &banner, const string &fileNote, const string &date, const string &user, const string &encoding)
{
}

void DSLConverter::endHeader()
{
}

void DSLConverter::startModel(const string &projectName, const string &name)
{
    m_groups.push_back(name);
    m_groupsChildren.push_back(vector<string>());
}

void DSLConverter::endModel()
{
    writer->writeGroup(m_groups.back(), m_groupsChildren.back());
    // Export ?
}

void DSLConverter::startGroup(const std::string &name, const Vector3F &translation, const int &materialId)
{
    string dslName = name;

    size_t p;
    while ((p = dslName.find_first_of(' ')) != string::npos)
        dslName[p] = '_';
    while ((p = dslName.find_first_of('-')) != string::npos)
        dslName[p] = '_';
    while ((p = dslName.find_first_of('/')) != string::npos)
    {
        dslName[p] = '_';
    }

    m_groups.push_back(dslName);
    m_groupsChildren.push_back(vector<string>());
    m_groupsTranslation.push_back(translation);
}

void DSLConverter::endGroup()
{
    if (m_groupsChildren.back().size())
    {
        writer->writeGroup(m_groups.back(), m_groupsChildren.back());
        m_groupsChildren.pop_back();
        m_groupsChildren.back().push_back(m_groups.back());
        m_groups.pop_back();
    }
    else
    {
        m_groupsChildren.pop_back();
        m_groups.pop_back();
    }
}

void DSLConverter::startMetaData()
{
}

void DSLConverter::endMetaData()
{
}

void DSLConverter::startMetaDataPair(const string &name, const string &value)
{
}

void DSLConverter::endMetaDataPair()
{
}

void DSLConverter::createPyramid(const std::array<float, 12> &matrix, const Primitives::Pyramid &params)
{
    string shapeid = std::to_string(m_lastShapeId++);
    writer->writePyramid("bshape" + shapeid, params.xbottom(), params.ybottom(), params.xtop(), params.ytop(), params.height(), params.xoffset(), params.yoffset());
    writeShapeTransforms(shapeid, matrix);
}

void DSLConverter::createBox(const std::array<float, 12> &matrix, const Primitives::Box &params)
{
    string shapeid = std::to_string(m_lastShapeId++);
    writer->writeBox("bshape" + shapeid, params.len[0], params.len[1], params.len[2]);
    writeShapeTransforms(shapeid, matrix);
}

void DSLConverter::createRectangularTorus(const std::array<float, 12> &matrix, const Primitives::RectangularTorus &params)
{
    string shapeid = std::to_string(m_lastShapeId++);
    writer->writeRectangularTorus("bshape" + shapeid, params.routside(), params.rinside(), params.height(), params.angle());
    writeShapeTransforms(shapeid, matrix);
}

void DSLConverter::createCircularTorus(const std::array<float, 12> &matrix, const Primitives::CircularTorus &params)
{
    string shapeid = std::to_string(m_lastShapeId++);
    writer->writeCircularTorus("bshape" + shapeid, params.radius(), params.offset(), params.angle());
    writeShapeTransforms(shapeid, matrix);
}

void DSLConverter::createEllipticalDish(const std::array<float, 12> &matrix, const Primitives::EllipticalDish &params)
{
    string shapeid = std::to_string(m_lastShapeId++);
    writer->writeDish("bshape" + shapeid, params.radius(), params.diameter(), params.radius());
    writeShapeTransforms(shapeid, matrix);
}

void DSLConverter::createSphericalDish(const std::array<float, 12> &matrix, const Primitives::SphericalDish &params)
{
    string shapeid = std::to_string(m_lastShapeId++);
    writer->writeDish("bshape" + shapeid, params.height(), params.diameter(), params.diameter());
    writeShapeTransforms(shapeid, matrix);
}

void DSLConverter::createSnout(const std::array<float, 12> &matrix, const Primitives::Snout &params)
{
    string shapeid = std::to_string(m_lastShapeId++);
    writer->writeSnout("bshape" + shapeid, params.dbottom() / 2, params.dtop() / 2, params.xoffset(), params.yoffset(), params.height());
    writeShapeTransforms(shapeid, matrix);
}

void DSLConverter::createCylinder(const std::array<float, 12> &matrix, const Primitives::Cylinder &params)
{
    string shapeid = std::to_string(m_lastShapeId++);
    writer->writeCylinder("bshape" + shapeid, params.radius(), params.height());
    writeShapeTransforms(shapeid, matrix);
}

void DSLConverter::createSphere(const std::array<float, 12> &matrix, const Primitives::Sphere &params)
{
    string shapeid = std::to_string(m_lastShapeId++);
    writer->writeSphere("bshape" + shapeid, params.diameter / 2);
    writeShapeTransforms(shapeid, matrix);
}

void DSLConverter::createLine(const std::array<float, 12> &matrix, const float &startx, const float &endx)
{
    string shapeid = std::to_string(m_lastShapeId++);
    writer->writeLine("bshape" + shapeid, startx, 0, 0, endx, 0, 0);
    writeShapeTransforms(shapeid, matrix);
}

void DSLConverter::createFacetGroup(const std::array<float, 12> &matrix, const vector<vector<vector<pair<Vector3F, Vector3F>>>> &vertexes)
{
    // Not supported.
}

void DSLConverter::writeShapeTransforms(const string &shapeid, const std::array<float, 12> &matrix)
{
    float rx, ry, rz;
    if (matrix[4] > 0.998)
    {
        ry = atan2(matrix[2], matrix[10]);
        rz = (float)M_PI / 2;
        rx = 0;
    }
    else if (matrix[4] < -0.998)
    {
        ry = atan2(matrix[2], matrix[10]);
        rz = (float)-M_PI / 2;
        rx = 0;
    }
    else
    {
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
