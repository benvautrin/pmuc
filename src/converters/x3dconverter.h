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

#ifndef X3DCONVERTER_H
#define X3DCONVERTER_H

#include "../api/rvmreader.h"
#include "../api/rvmmeshhelper.h"

#include <utility>
#include <map>

namespace XIOT {
    class X3DWriter;
}

typedef std::map<std::vector<float>, std::pair<std::string,int> > X3DInstanceMap;

class X3DConverter : public RVMReader
{
    public:
        X3DConverter(const std::string& filename, bool binary);
        virtual ~X3DConverter();

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

        virtual void createPyramid(const std::array<float, 12>& matrix, const Primitives::Pyramid& pyramid);

        virtual void createBox(const std::array<float, 12>& matrix, const Primitives::Box& parameters);

        virtual void createRectangularTorus(const std::array<float, 12>& matrix, const Primitives::RectangularTorus& parameters);

        virtual void createCircularTorus(const std::array<float, 12>& matrix, const Primitives::CircularTorus& parameters);

        virtual void createEllipticalDish(const std::array<float, 12>& matrix, const Primitives::EllipticalDish& parameters);

        virtual void createSphericalDish(const std::array<float, 12>& matrix, const Primitives::SphericalDish& parameters);

        virtual void createSnout(const std::array<float, 12>& matrix, const Primitives::Snout& parameters);

        virtual void createCylinder(const std::array<float, 12>& matrix, const Primitives::Cylinder& parameters);

        virtual void createSphere(const std::array<float, 12>& matrix, const Primitives::Sphere& parameters);

        virtual void createLine(const std::array<float, 12>& matrix, const float& startx, const float& endx);

        virtual void createFacetGroup(const std::array<float, 12>& matrix,
                                     const std::vector<std::vector<std::vector<std::pair<Vector3F, Vector3F> > > >& vertexes);

    private:
        void startShape(const std::array<float, 12>& matrix);
        void endShape();

        void startNode(int id);
        void endNode(int id);

        int startMeshGeometry(const Mesh& mesh, const std::string &id);
        void writeMeshInstance(int meshType, const std::string &use);

        void writeMetaDataString(const std::string &name, const std::string &value, bool isValue = false);
        std::pair<std::string, int> getInstanceName(const std::vector<float> &params);
        std::string createGeometryId();

        X3DInstanceMap m_instanceMap;
        int m_id;

        std::vector<XIOT::X3DWriter*> m_writers;
        std::vector<Vector3F> m_translations;
        std::vector<int> m_materials;
        std::vector<std::string> m_groups;
        bool m_binary;
        std::vector<int> m_nodeStack;
};

#endif // X3DCONVERTER_H
