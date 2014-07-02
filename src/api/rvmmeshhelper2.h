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

#ifndef RVMMESHHELPER2_H
#define RVMMESHHELPER2_H

#include <utility>
#include <vector>

#include "vector3f.h"

struct Mesh {
 std::vector<unsigned long>  positionIndex;
 std::vector<unsigned long>  normalIndex;
 std::vector<Vector3F>  positions;
 std::vector<Vector3F>  normals;
};

class RVMMeshHelper2
{
    public:
        RVMMeshHelper2();

    public:
        /**
         * @brief Builds up indexed coordinates for the described pyramid
         * @param xbottom
         * @param ybottom
         * @param xtop
         * @param ytop
         * @param xoffset
         * @param yoffset
         * @param height
         * @param maxSideSize not used here. For consistency with the other methods.
         * @param minSides not used here. For consistency with the other methods.
         * @return vertexes coordinates and their index.
         */
        static const Mesh makePyramid(const float& xbottom, const float& ybottom, const float& xtop, const float& ytop, const float& xoffset, const float& yoffset, const float& height, const float& maxSideSize, const int& minSides);

        /**
         * @brief Builds up indexed coordinates for the described box
         * @param x
         * @param y
         * @param z
         * @param maxSideSize not used here. For consistency with the other methods.
         * @param minSides not used here. For consistency with the other methods.
         * @return vertexes coordinates and their index.
         */
        static const Mesh makeBox(const float& x, const float& y, const float& z, const float& maxSideSize, const int& minSides);

        /**
         * @brief Builds up a sphere with the given radius
         * @param radius
         * @param maxSideSize
         * @param minSides
         * @return coordinates and normals with their indexes.
         */
        static const Mesh makeSphere(const float& radius, const float& maxSideSize, const int& minSides);

        /**
         * @brief makeCylinder
         * @param radius
         * @param height
         * @param maxSideSize
         * @param minSides
         * @return
         */
        static const Mesh makeCylinder(const float& radius, const float& height, const float& maxSideSize, const int& minSides);

        /**
         * @brief makeRectangularTorus
         * @param rinside
         * @param routside
         * @param height
         * @param angle
         * @param maxSideSize
         * @param minSides
         * @return
         */
        static const Mesh makeRectangularTorus(const float& rinside, const float& routside, const float& height, const float& angle, const float& maxSideSize, const int& minSides);

        /**
         * @brief makeCircularTorus
         * @param rinside
         * @param routside
         * @param angle
         * @param maxSideSize
         * @param minSides
         * @return
         */
        static const Mesh makeCircularTorus(const float& rinside, const float& routside, const float& angle, const float& maxSideSize, const int& minSides);

        /**
         * @brief makeSnout
         * @param rbottom
         * @param rtop
         * @param height
         * @param xoffset
         * @param yoffset
         * @param maxSideSize
         * @param minSides
         * @return
         */
        static const Mesh makeSnout(const float& rbottom, const float& rtop, const float& height, const float& xoffset, const float& yoffset, const float& maxSideSize, const int& minSides);

        /**
         * @brief makeEllipticalDish
         * @param diameter
         * @param radius
         * @param maxSideSize
         * @param minSides
         * @return
         */
        static const Mesh makeEllipticalDish(const float& dishradius, const float& secondradius, const float& maxSideSize, const int& minSides);

        /**
         * @brief makeSphericalDish
         * @param diameter
         * @param height
         * @param maxSideSize
         * @param minSides
         * @return
         */
        static const Mesh makeSphericalDish(const float& dishradius, const float& height, const float& maxSideSize, const int& minSides);
};

#endif // RVMMESHHELPER_H
