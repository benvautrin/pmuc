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

        virtual void startPyramid(const std::vector<float>& matrix,
                                  const float& xbottom,
                                  const float& ybottom,
                                  const float& xtop,
                                  const float& ytop,
                                  const float& height,
                                  const float& xoffset,
                                  const float& yoffset);
        virtual void endPyramid();

        virtual void startBox(const std::vector<float>& matrix,
                              const float& xlength,
                              const float& ylength,
                              const float& zlength);
        virtual void endBox();

        virtual void startRectangularTorus(const std::vector<float>& matrix,
                                           const float& rinside,
                                           const float& routside,
                                           const float& height,
                                           const float& angle);
        virtual void endRectangularTorus();

        virtual void startCircularTorus(const std::vector<float>& matrix,
                                        const float& rinside,
                                        const float& routside,
                                        const float& angle);
        virtual void endCircularTorus();

        virtual void startEllipticalDish(const std::vector<float>& matrix,
                                         const float& diameter,
                                         const float& radius);
        virtual void endEllipticalDish();

        virtual void startSphericalDish(const std::vector<float>& matrix,
                                        const float& diameter,
                                        const float& height);
        virtual void endSphericalDish();

        virtual void startSnout(const std::vector<float>& matrix,
                                const float& dtop,
                                const float& dbottom,
                                const float& height,
                                const float& xoffset,
                                const float& yoffset,
                                const float& unknown1,
                                const float& unknown2,
                                const float& unknown3,
                                const float& unknown4);
        virtual void endSnout();

        virtual void startCylinder(const std::vector<float>& matrix,
                                   const float& radius,
                                   const float& height);
        virtual void endCylinder();

        virtual void startSphere(const std::vector<float>& matrix,
                                 const float& diameter);
        virtual void endSphere();

        virtual void startLine(const std::vector<float>& matrix,
                               const float& startx,
                               const float& endx);
        virtual void endLine();

        virtual void startFacetGroup(const std::vector<float>& matrix,
                                     const std::vector<std::vector<std::vector<std::pair<Vector3F, Vector3F> > > >& vertexes);
        virtual void endFacetGroup();

    private:
        void startShape(const std::vector<float>& matrix);
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
