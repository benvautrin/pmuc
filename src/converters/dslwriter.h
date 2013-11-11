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

#ifndef DSLWRITER_H
#define DSLWRITER_H

#include <string>
#include <vector>

#include <stdio.h>

class DSLWriter
{
    public:
        DSLWriter();

        bool open(const std::string& filename);
        void close();

        void writeTranslation(const std::string& newId, const std::string& objectId, float x, float y, float z);
        void writeRotation(const std::string& newId, const std::string& objectId, float x, float y, float z);

        void writeSphere(const std::string& id, float radius);
        void writeBox(const std::string& id, float lx, float ly, float lz);
        void writeCone(const std::string& id, float major_radius, float minor_radius, float height);
        void writeSnout(const std::string& id, float major_radius, float minor_radius, float xoff, float yoff, float height);
        void writeWedge(const std::string& id, float x, float y, float z, float angle);
        void writeCircularTorus(const std::string& id, float r_outside, float r_inside, float angle);
        void writeRectangularTorus(const std::string& id, float r_outside, float r_inside, float height, float angle);
        void writeCylinder(const std::string& id, float radius, float height);
        void writeDish(const std::string& id, float height, float diameter, float angle);
        void writePyramid(const std::string& id, float lx_bottom, float ly_bottom, float lx_top, float ly_top, float height, float xoff, float yoff);
        void writeNozzle(const std::string& id, float height, float r_inside, float r_outside, float nozzle_height, float nozzle_radius);
        void writeLine(const std::string& id, float x1, float y1, float z1, float x2, float y2, float z2);

        void writeGroup(const std::string& id, const std::vector<std::string>& children);

    private:
        FILE* fp;
};

#endif // DSLWRITER_H
