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

#ifndef COLLADACONVERTER_H
#define COLLADACONVERTER_H

#include <COLLADASWStreamWriter.h>

#include "../api/rvmreader.h"
#include "../api/rvmmeshhelper.h"

class CCModel;

typedef std::map<std::vector<float>, std::string> InstanceMap;

class COLLADAConverter : public RVMReader
{
    public:
        COLLADAConverter(const std::string& filename);
        virtual ~COLLADAConverter();

        virtual void startDocument();
        virtual void endDocument();

        virtual void startHeader(const std::string& banner, const std::string& fileNote, const std::string& date, const std::string& user, const std::string& encoding);
        virtual void endHeader();

        virtual void startModel(const std::string& projectName, const std::string& name);
        virtual void endModel();

        virtual void startGroup(const std::string& name, const Vector3F& translation, const int& materialId);
        virtual void endGroup();

        virtual void startMetaData();
        virtual void endMetaData();

        virtual void startMetaDataPair(const std::string& name, const std::string& value);
        virtual void endMetaDataPair();

        virtual void createPyramid(const std::array<float, 12>& matrix, const Primitives::Pyramid& params);

        virtual void createBox(const std::array<float, 12>& matrix, const Primitives::Box& params);

        virtual void createRectangularTorus(const std::array<float, 12>& matrix, const Primitives::RectangularTorus& params);

        virtual void createCircularTorus(const std::array<float, 12>& matrix, const Primitives::CircularTorus& params);

        virtual void createEllipticalDish(const std::array<float, 12>& matrix, const Primitives::EllipticalDish& params);

        virtual void createSphericalDish(const std::array<float, 12>& matrix, const Primitives::SphericalDish& params);

        virtual void createSnout(const std::array<float, 12>& matrix, const Primitives::Snout& params);

        virtual void createCylinder(const std::array<float, 12>& matrix, const Primitives::Cylinder& params);

        virtual void createSphere(const std::array<float, 12>& matrix, const Primitives::Sphere& params);

        virtual void createLine(const std::array<float, 12>& matrix, const float& startx, const float& endx);

        virtual void createFacetGroup(const std::array<float, 12>& matrix,
                                     const std::vector<std::vector<std::vector<std::pair<Vector3F, Vector3F> > > >& vertexes);

    private:

        void writeMesh(const std::string &gid, const Mesh& mesh, const std::string comment = "");
        void addGeometry(const std::string &gid, const std::array<float, 12> &matrix);
        std::string getInstanceName(const std::vector<float> &params);
        std::string createGeometryId();

        COLLADASW::StreamWriter* m_writer;
        std::vector<Vector3F> m_translations;
        InstanceMap m_instanceMap;
        CCModel* m_model;
};

#endif // COLLADACONVERTER_H
