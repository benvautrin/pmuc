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

#ifndef IFCCONVERTER_H
#define IFCCONVERTER_H

#include "../api/rvmreader.h"
#include "../api/rvmmeshhelper.h"

#include <ifcpp/model/IfcPPModel.h>
#include <ifcpp/model/StatusCallback.h>

#include <Eigen/Core>

#include <stack>

class IfcOwnerHistory;
class IfcRelAggregates;
class IfcLocalPlacement;
class IfcObjectDefinition;
class IfcGeometricRepresentationContext;
class IfcRepresentationItem;
class IfcMaterial;
class IfcPropertySet;
class IfcLabel;
class IfcSurfaceStyle;
class IfcPPEntity;
class IfcAxis2Placement3D;
class IfcProfileDef;
class IfcDirection;
class IfcPlane;

typedef Eigen::Transform<float, 3, Eigen::Affine> Transform3f;

class IFCConverter : public RVMReader
{
    public:
        IFCConverter(const std::string& filename, const std::string& schema);
        virtual ~IFCConverter();

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

        virtual void createFacetGroup(const std::array<float, 12>& matrix,  const FGroup& vertexes);

        static void messageCallBack(void* obj_ptr, shared_ptr<StatusCallback::Message> t);

    private:
        shared_ptr<IfcPPModel>                          m_model;
        shared_ptr<IfcOwnerHistory>                     m_owner_history;
        shared_ptr<IfcGeometricRepresentationContext>   m_context;
        std::string                                     m_filename;
        std::stack<shared_ptr<IfcRelAggregates> >       m_relationStack;
        std::map<int, shared_ptr<IfcMaterial> >         m_materials;
        std::map<int, shared_ptr<IfcSurfaceStyle> >     m_styles;
        shared_ptr<IfcPropertySet>                      m_propertySet;
        int                                             m_currentEntityId;
        std::stack<int>                                 m_currentMaterial;

        shared_ptr<IfcOwnerHistory> createOwnerHistory(const std::string &name, const std::string &banner, int timeStamp);
        shared_ptr<IfcMaterial> createMaterial(int id);
        shared_ptr<IfcSurfaceStyle> createSurfaceStyle(int id);
        void createSlopedCylinder(const std::array<float, 12>& matrix, const Primitives::Snout& params);
        void insertEntity(shared_ptr<IfcPPEntity> e);
        void initModel();
        void pushParentRelation(shared_ptr<IfcObjectDefinition> parent);
        void addRepresentationToShape(shared_ptr<IfcRepresentationItem> item, shared_ptr<IfcLabel> type);
        void addRevolvedAreaSolidToShape(shared_ptr<IfcProfileDef> profile, shared_ptr<IfcDirection> axis, double angle, const Transform3f& transform);

        void writeMesh(const Mesh &mesh, const std::array<float, 12>& matrix);
        shared_ptr<IfcAxis2Placement3D> getCoordinateSystem(const Transform3f& matrix, const Eigen::Vector3f &offset);
        shared_ptr<IfcPlane> createClippingPlane(double zPos, const Eigen::Vector3d &normal);
};

#endif // IFCCONVERTER_H
