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

#ifndef RVMREADER_H
#define RVMREADER_H

#include <string>
#include <vector>
#include <array>

#include "vector3f.h"
#include "rvmprimitive.h"

/**
 * @brief RVM reader base class
 *
 * Implement this class to use the data found by the RVM parser.
 * Returns data as described in the document written by Kristian Sons
 *
 * @see X3DConverter @see DummyReader
 */
class RVMReader
{
    public:
        /**
         * @brief Default constructor. Initializes variables.
         */
        RVMReader();
        /**
         * @brief Pure virtual destructor
         */
        virtual ~RVMReader() = 0;

        /**
         * @brief Signals the start of the document.
         */
        virtual void startDocument() = 0;
        /**
         * @brief Signals the end of the document.
         */
        virtual void endDocument() = 0;

        /**
         * @brief Called when the header of the RVM file is found
         * @param banner Name and version of PDMS that produced the RVM data.
         * @param fileNote notes concerning the file
         * @param date string describing the date of export.
         * @param user the login of the user that created the data.
         * @param encoding only in version 2 files, string encoding.
         */
        virtual void startHeader(const std::string& banner, const std::string& fileNote, const std::string& date, const std::string& user, const std::string& encoding) = 0;
        /**
         * @brief Called at the end of the header.
         */
        virtual void endHeader() = 0;

        /**
         * @brief Called at the start of the model
         * @param projectName the name of the project
         * @param name the name of the model.
         */
        virtual void startModel(const std::string& projectName, const std::string& name) = 0;
        /**
         * @brief Called at the end of the model.
         */
        virtual void endModel() = 0;

        /**
         * @brief Called at the start of a RVM group
         * @param name the name of the group
         * @param translation the translation of the group, relative to the model origin.
         * @param materialId the material to use in the form of a PDMS color index.
         * @see RVMColorHelper
         */
        virtual void startGroup(const std::string& name, const float translation[3], const int& materialId) = 0;
        /**
         * @brief Called at the end of a group.
         */
        virtual void endGroup() = 0;

        /**
         * @brief Called if attributes are found for an group.
         */
        virtual void startMetaData() = 0;
        /**
         * @brief Called at the end of the attributes.
         */
        virtual void endMetaData() = 0;

        /**
         * @brief Called for each key/value attribute pair
         * @param name the name of the attribute.
         * @param value its value.
         */
        virtual void startMetaDataPair(const std::string& name, const std::string& value) = 0;
        /**
         * @brief Called at the end of an attribute.
         */
        virtual void endMetaDataPair() = 0;

        /**
         * @brief Describes a pyramid in a group
         * @param matrix 3x4 transformation matrix
         * @param pyramid
         * @see RVMMeshHelper::makePyramid
         */
        virtual void createPyramid(const std::array<float, 12>& matrix, const Primitives::Pyramid& pyramid) = 0;

        /**
         * @brief Describes a box in a group
         * @param matrix 3x4 transformation matrix
         * @param len
         * @see RVMMeshHelper::makeBox
         */
        virtual void createBox(const std::array<float, 12>& matrix, const Primitives::Box& box) = 0;

        /**
         * @brief Describes a rectangular torus
         * @param matrix 3x4 transformation matrix
         * @param rTorus
         * @see RVMMeshHelper::makeRectangularTorus
         */
        virtual void createRectangularTorus(const std::array<float, 12>& matrix, const Primitives::RectangularTorus& rTorus) = 0;

        /**
         * @brief Describes a circular torus
         * @param matrix
         * @param cTorus
         * @see RVMMeshHelper::makeCircularTorus
         */
        virtual void createCircularTorus(const std::array<float, 12>& matrix, const Primitives::CircularTorus& cTorus) = 0;

        /**
         * @brief Describes an elliptical dish
         * @param matrix
         * @param eDish
         * @see RVMMeshHelper::makeEllipticalDish
         */
        virtual void createEllipticalDish(const std::array<float, 12>& matrix, const Primitives::EllipticalDish& eDish) = 0;

        /**
         * @brief Describes an spherical dish
         * @param matrix
         * @param sDish
         * @see RVMMeshHelper::makeSphericalDish
         */
        virtual void createSphericalDish(const std::array<float, 12>& matrix, const Primitives::SphericalDish& sDish) = 0;

        /**
         * @brief Describes a snout.
         * @param matrix
         * @param snout
         * @see RVMMeshHelper::makeSnout
         */
        virtual void createSnout(const std::array<float, 12>& matrix, const Primitives::Snout& snout) = 0;

        /**
         * @brief Describes a cylinder
         * @param matrix
         * @param cylinder
         * @see RVMMeshHelper::makeCylinder
         */
        virtual void createCylinder(const std::array<float, 12>& matrix, const Primitives::Cylinder& cylinder) = 0;

        /**
         * @brief Describes a sphere
         * @param matrix
         * @param sphere
         * @see RVMMeshHelper::makeSphere
         */
        virtual void createSphere(const std::array<float, 12>& matrix, const Primitives::Sphere& sphere) = 0;

        /**
         * @brief Describes a line
         * @param matrix
         * @param startx
         * @param endx
         */
        virtual void startLine(const std::array<float, 12>& matrix,
                               const float& startx,
                               const float& endx) = 0;
        /**
         * @brief End of a line
         */
        virtual void endLine() = 0;

        /**
         * @brief Describes a facet group
         *
         * Separated in patch/group/vertexes.
         * If more than one group is found in a patch, each group should be closed.
         *
         * @param matrix
         * @param vertexes
         */
        virtual void startFacetGroup(const std::array<float, 12>& matrix,
                                     const std::vector<std::vector<std::vector<std::pair<Vector3F, Vector3F> > > >& vertexes) = 0;
        /**
         * @brief End of a facet group
         */
        virtual void endFacetGroup() = 0;

        /**
         * @brief Sets the maximum size for a side of a primitive when tesselating.
         * @param size
         */
        void setMaxSideSize(float size) { m_maxSideSize = size; }
        /**
         * @brief Sets the minimum number of sides of a tesselated primitive.
         * @param number
         */
        void setMinSides(int number) { m_minSides = number; }
        /**
         * @brief Sets if the user wants the data to be split with a file for each group.
         * @param split
         */
        void setSplit(bool split) { m_split = split; }
        /**
         * @brief Sets if the reader should use native primitives instead of tesselating.
         * @param primitives
         */
        void setUsePrimitives(bool primitives) { m_primitives = primitives; }

    protected:
        int m_minSides;
        float m_maxSideSize;
        bool m_split;
        bool m_primitives;
};

#endif // RVMREADER_H
