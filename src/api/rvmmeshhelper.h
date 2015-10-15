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
#include <algorithm>

#include "vector3f.h"
#include "rvmprimitive.h"

struct Mesh {
 std::vector<unsigned long>  positionIndex;
 std::vector<unsigned long>  normalIndex;
 std::vector<Vector3F>  positions;
 std::vector<Vector3F>  normals;
};


enum PrimitiveTypes {
    Pyramid = 1,
    Box = 2,
    RectangularTorus = 3,
    CircularTorus = 4,
    EllipticalDish = 5,
    SphericalDish = 6,
    Snout = 7,
    Cylinder =  8,
    Sphere = 9,
    Line = 10,
    FacetGroup = 11
};


typedef std::pair<Vector3F, Vector3F> Vertex;

class RVMMeshHelper2
{
    public:
        RVMMeshHelper2();

    public:
        /**
         * @brief Builds up indexed coordinates for the described pyramid
         * @param inP
         * @param maxSideSize not used here. For consistency with the other methods.
         * @param minSides not used here. For consistency with the other methods.
         * @return vertexes coordinates and their index.
         */
        static const Mesh makePyramid(const Primitives::Pyramid& inP, const float& maxSideSize, const int& minSides);

        /**
         * @brief Builds up indexed coordinates for the described box
         * @param x
         * @param y
         * @param z
         * @param maxSideSize not used here. For consistency with the other methods.
         * @param minSides not used here. For consistency with the other methods.
         * @return vertexes coordinates and their index.
         */
        static const Mesh makeBox(const Primitives::Box& box, const float& maxSideSize, const int& minSides);

        /**
         * @brief Builds up a sphere with the given radius
         * @param radius
         * @param maxSideSize
         * @param minSides
         * @return coordinates and normals with their indexes.
         */
        static const Mesh makeSphere(const Primitives::Sphere &sphere, const float& maxSideSize, const int& minSides);

        /**
         * @brief makeCylinder
         *
         * @param cylinder The description for the cylinder primitive.
         * @param sides    The number of cylinder sides, can be computed with infoCylinderNumSides.
         *
         * @return Returns a mesh object.
         */
        static const Mesh makeCylinder(const Primitives::Cylinder &cylinder, unsigned long sides);

        /**
         * @brief makeRectangularTorus
         * @param rt
         * @param maxSideSize
         * @param minSides
         * @return
         */
        static const Mesh makeRectangularTorus(const Primitives::RectangularTorus& rt, const float& maxSideSize, const int& minSides);

        /**
         * @brief makeCircularTorus
         * @param rinside
         * @param routside
         * @param angle
         * @param maxSideSize
         * @param minSides
         * @return
         */
        static const Mesh makeCircularTorus(const Primitives::CircularTorus& cTorus, unsigned long tsides, unsigned long csides);

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
        static const Mesh makeSnout(const Primitives::Snout& snout, unsigned long sides);

        /**
         * @brief makeEllipticalDish
         * @param diameter
         * @param radius
         * @param sides
         * @param csides
         * @return
         */
        static const Mesh makeEllipticalDish(const Primitives::EllipticalDish& eDish, unsigned long sides, unsigned long csides);

        /**
         * @brief makeSphericalDish
         * @param diameter
         * @param height
         * @param maxSideSize
         * @param minSides
         * @return
         */
        static const Mesh makeSphericalDish(const Primitives::SphericalDish& sDish , const float& maxSideSize, const int& minSides);


        static void tesselateFacetGroup(const std::vector<std::vector<std::vector<Vertex> > >& vertices, Mesh* meshData);

        /**
         * @param cylinder The cylinder primitive data.
         * @param maxSideSize
         * @param minSides
         *
         * @return Returns the number of sides of the cylinder.
         */

        static unsigned long infoCylinderNumSides(const Primitives::Cylinder &cylinder, float maxSideSize, unsigned long minSides);
        static unsigned long infoSnoutNumSides(const Primitives::Snout &snout, float maxSideSize, unsigned long minSides);

        static std::pair<unsigned long, unsigned long> infoCircularTorusNumSides(const Primitives::CircularTorus& cTorus, float maxSideSize, unsigned long minSides);
        static std::pair<unsigned long, unsigned long> infoEllipticalDishNumSides(const Primitives::EllipticalDish& eDish, float maxSideSize, unsigned long minSides);
};

#endif // RVMMESHHELPER_H
