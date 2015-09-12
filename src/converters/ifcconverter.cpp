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

#define BOOST_DATE_TIME_NO_LIB
#include <boost/algorithm/string/replace.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/posix_time/posix_time_io.hpp>
#include <boost/locale/encoding_utf.hpp>

#include <ifcpp/writer/IfcPPWriterSTEP.h>
#include <ifcpp/IFC4/include/IfcProject.h>
#include <ifcpp/IFC4/include/IfcPerson.h>
#include <ifcpp/IFC4/include/IfcIdentifier.h>
#include <ifcpp/IFC4/include/IfcOwnerHistory.h>
#include <ifcpp/IFC4/include/IfcPersonAndOrganization.h>
#include <ifcpp/IFC4/include/IfcLabel.h>
#include <ifcpp/IFC4/include/IfcGloballyUniqueId.h>
#include <ifcpp/IFC4/include/IfcBuilding.h>
#include <ifcpp/IFC4/include/IfcSite.h>
#include <ifcpp/IFC4/include/IfcCartesianPoint.h>
#include <ifcpp/IFC4/include/IfcAxis2Placement3D.h>
#include <ifcpp/IFC4/include/IfcLengthMeasure.h>
#include <ifcpp/IFC4/include/IfcRelAggregates.h>
#include <ifcpp/IFC4/include/IfcBuildingElementProxy.h>
#include <ifcpp/IFC4/include/IfcProductDefinitionShape.h>
#include <ifcpp/IFC4/include/IfcProduct.h>
#include <ifcpp/IFC4/include/IfcShapeRepresentation.h>
#include <ifcpp/IFC4/include/IfcGeometricRepresentationContext.h>
#include <ifcpp/IFC4/include/IfcDimensionCount.h>
#include <ifcpp/IFC4/include/IfcFaceBasedSurfaceModel.h>
#include <ifcpp/IFC4/include/IfcConnectedFaceSet.h>
#include <ifcpp/IFC4/include/IfcPolyLoop.h>
#include <ifcpp/IFC4/include/IfcFaceOuterBound.h>
#include <ifcpp/IFC4/include/IfcFace.h>
#include <ifcpp/IFC4/include/IfcUnitAssignment.h>
#include <ifcpp/IFC4/include/IfcSIUnit.h>
#include <ifcpp/IFC4/include/IfcSIUnitName.h>
#include <ifcpp/IFC4/include/IfcUnitEnum.h>
#include <ifcpp/IFC4/include/IfcLocalPlacement.h>
#include <ifcpp/IFC4/include/IfcTriangulatedFaceSet.h>
#include <ifcpp/IFC4/include/IfcCartesianPointList3D.h>
#include <ifcpp/IFC4/include/IfcLengthMeasure.h>
#include <ifcpp/IFC4/include/IfcMaterial.h>
#include <ifcpp/IFC4/include/IfcRelAssociatesMaterial.h>
#include <ifcpp/IFC4/include/IfcMaterialDefinitionRepresentation.h>
#include <ifcpp/IFC4/include/IfcStyledRepresentation.h>
#include <ifcpp/IFC4/include/IfcStyledItem.h>
#include <ifcpp/IFC4/include/IfcPresentationStyleAssignment.h>
#include <ifcpp/IFC4/include/IfcColourRgb.h>
#include <ifcpp/IFC4/include/IfcNormalisedRatioMeasure.h>
#include <ifcpp/IFC4/include/IfcSurfaceStyleRendering.h>
#include <ifcpp/IFC4/include/IfcSurfaceStyle.h>
#include <ifcpp/IFC4/include/IfcSurfaceSide.h>
#include <ifcpp/IFC4/include/IfcPropertySet.h>
#include <ifcpp/IFC4/include/IfcRelDefinesByProperties.h>
#include <ifcpp/IFC4/include/IfcPropertySingleValue.h>
#include <ifcpp/IFC4/include/IfcText.h>

#include <ifcpp/model/IfcPPGuid.h>

#include <Eigen/Core>

#include "../api/rvmmeshhelper.h"
#include "../api/rvmcolorhelper.h"

#include <fstream>
#include <string>
#include <cassert>

using boost::locale::conv::utf_to_utf;

namespace {
    shared_ptr<IfcNormalisedRatioMeasure> getNormalisedRatioMeasure(double value) {
        shared_ptr<IfcNormalisedRatioMeasure> measure( new IfcNormalisedRatioMeasure() );
        measure->m_value = value;
        return measure;
    }

    std::wstring utf8_to_wstring(const std::string& str)
    {
        return utf_to_utf<wchar_t>(str.c_str(), str.c_str() + str.size());
    }

    Eigen::Matrix4f toEigenMatrix(const std::array<float, 12>& matrix) {
        Eigen::Matrix4f result;
        result.setIdentity();
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 3; j++) {
                result(j, i)= matrix[i*3+j];
            }
        }
        return result;
    }
}


IFCConverter::IFCConverter(const std::string& filename, const std::string& schema) : RVMReader(), m_filename(filename), m_model(new IfcPPModel()), m_currentEntityId(0) {
    std::string filename_escaped = boost::replace_all_copy(filename, "\\", "\\\\");
	std::wstringstream strs;
	strs << "ISO-10303-21;" << std::endl;
	strs << "HEADER;" << std::endl;
	strs << "FILE_DESCRIPTION(('PMUC generated IFC file.'),'2;1');" << std::endl;
	strs << "FILE_NAME('" << filename_escaped.c_str() << "','";

	//2011-04-21T14:25:12
	std::locale loc( std::wcout.getloc(), new boost::posix_time::wtime_facet( L"%Y-%m-%dT%H:%M:%S" ) );
	std::basic_stringstream<wchar_t> wss;
	wss.imbue( loc );
	boost::posix_time::ptime now = boost::posix_time::second_clock::local_time();
	wss << now;
	std::wstring ts = wss.str().c_str();

	strs << ts;
	strs << "',(''),('',''),'','IfcPlusPlus','');" << std::endl;
    strs << "FILE_SCHEMA(('" << schema.c_str() << "'));" << std::endl;
	strs << "ENDSEC;" << std::endl;

    m_model->setFileHeader(strs.str().c_str());
}

IFCConverter::~IFCConverter() {
}

void IFCConverter::startDocument() {
    shared_ptr<IfcProject> project(new IfcProject());
    insertEntity(project);
    project->m_GlobalId = shared_ptr<IfcGloballyUniqueId>(new IfcGloballyUniqueId( CreateCompressedGuidString22() ) );
    m_model->setIfcProject(project);

}

void IFCConverter::endDocument() {
    shared_ptr<IfcPPWriterSTEP> writer( new IfcPPWriterSTEP() );
    writer->setMessageCallBack(this, &IFCConverter::messageCallBack);
    std::stringstream stream;
    std::ofstream fileStream(m_filename);

    writer->writeModelToStream(stream, m_model);
    fileStream << stream.rdbuf();
    fileStream.close();
    std::cout << std::endl; // Finish progress line
}

void IFCConverter::startHeader(const std::string& banner, const std::string& fileNote, const std::string& date, const std::string& user, const std::string& encoding) {
    m_model->getIfcProject()->m_OwnerHistory = createOwnerHistory(user);
}

void IFCConverter::endHeader() {

}

void IFCConverter::startModel(const std::string& projectName, const std::string& name) {
    // TODO(ksons): Which is long name and which is name
    m_model->getIfcProject()->m_Name = shared_ptr<IfcLabel>(new IfcLabel(std::wstring(projectName.begin(), projectName.end())));
    m_model->getIfcProject()->m_LongName = shared_ptr<IfcLabel>(new IfcLabel(std::wstring(name.begin(), name.end())));
    m_model->getIfcProject()->m_Phase = shared_ptr<IfcLabel>(new IfcLabel(L"$"));

    // Initialize model with site, building etc
    initModel();


}

void IFCConverter::endModel() {
    assert(m_relationStack.size() == 1);
    shared_ptr<IfcRelAggregates> aggregates = m_relationStack.top();

    // Do not add aggregates for empty model
    if(aggregates->m_RelatedObjects.size() > 0) {
        insertEntity(aggregates);
    }
    m_relationStack.pop();
}

void IFCConverter::startGroup(const std::string& name, const Vector3F& translation, const int& materialId) {
    shared_ptr<IfcBuildingElementProxy> buildingElement( new IfcBuildingElementProxy() );
    buildingElement->m_GlobalId = shared_ptr<IfcGloballyUniqueId>(new IfcGloballyUniqueId( CreateCompressedGuidString22() ) );
    buildingElement->m_Name = shared_ptr<IfcLabel>(new IfcLabel(std::wstring(name.begin(), name.end())));
    insertEntity(buildingElement);

    shared_ptr<IfcMaterial> material = createMaterial(materialId);
    shared_ptr<IfcRelAssociatesMaterial> materialAssociates( new IfcRelAssociatesMaterial() );
    materialAssociates->m_GlobalId = shared_ptr<IfcGloballyUniqueId>(new IfcGloballyUniqueId( CreateCompressedGuidString22() ) );
    materialAssociates->m_OwnerHistory = m_owner_history;
    materialAssociates->m_RelatedObjects.push_back(buildingElement);
    materialAssociates->m_RelatingMaterial = material;
    insertEntity(materialAssociates);

    // Build relation with parent group
    m_relationStack.top()->m_RelatedObjects.push_back(buildingElement);

    // Make this group top of the stack
    pushParentRelation(buildingElement);
}

void IFCConverter::endGroup() {
    shared_ptr<IfcRelAggregates> aggregates = m_relationStack.top();

    // Do not add aggregates for empty groups
    if(aggregates->m_RelatedObjects.size() > 0) {
        insertEntity(aggregates);
    }
    m_relationStack.pop();
}

void IFCConverter::startMetaData() {
    // Create a property set that holds all upcoming properties
    m_propertySet = shared_ptr<IfcPropertySet>( new IfcPropertySet() );
    m_propertySet->m_GlobalId = shared_ptr<IfcGloballyUniqueId>(new IfcGloballyUniqueId( CreateCompressedGuidString22() ) );
    m_propertySet->m_OwnerHistory = m_owner_history;
    m_propertySet->m_Description = shared_ptr<IfcText>( new IfcText( L"RVM Attributes" ));
    insertEntity(m_propertySet);

    // A related object is required (typically a IfcBuildingElementProxy)
    shared_ptr<IfcObjectDefinition> relatedObject = m_relationStack.top()->m_RelatingObject;
    assert(relatedObject);

    // Now link the created property set with the object
    shared_ptr<IfcRelDefinesByProperties> propertyRelation( new IfcRelDefinesByProperties() );
    propertyRelation->m_GlobalId = shared_ptr<IfcGloballyUniqueId>(new IfcGloballyUniqueId( CreateCompressedGuidString22() ) );
    propertyRelation->m_RelatedObjects.push_back(relatedObject);
    propertyRelation->m_RelatingPropertyDefinition = m_propertySet;
    propertyRelation->m_OwnerHistory = m_owner_history;
    insertEntity(propertyRelation);

}

void IFCConverter::endMetaData() {

}

void IFCConverter::startMetaDataPair(const std::string &name, const std::string &value) {
    assert(m_propertySet);
    std::string value_escaped = boost::replace_all_copy(value, "\\", "\\\\");

    shared_ptr<IfcPropertySingleValue> prop( new IfcPropertySingleValue() );
    insertEntity(prop);
    prop->m_Name = shared_ptr<IfcIdentifier>( new IfcIdentifier( utf8_to_wstring(name) ));
    prop->m_NominalValue = shared_ptr<IfcLabel>(new IfcLabel( utf8_to_wstring(value_escaped) ));

    m_propertySet->m_HasProperties.push_back(prop);
}

void IFCConverter::endMetaDataPair() {
}

void IFCConverter::createPyramid(const std::array<float, 12>& matrix, const Primitives::Pyramid& params) {
    writeMesh(RVMMeshHelper2::makePyramid(params, m_maxSideSize, m_minSides), matrix);
}


void IFCConverter::createBox(const std::array<float, 12>& matrix, const Primitives::Box& params) {
    writeMesh(RVMMeshHelper2::makeBox(params, m_maxSideSize, m_minSides), matrix);
}

void IFCConverter::createRectangularTorus(const std::array<float, 12>& matrix, const Primitives::RectangularTorus& params) {
    writeMesh(RVMMeshHelper2::makeRectangularTorus(params, m_maxSideSize, m_minSides), matrix);
}

void IFCConverter::createCircularTorus(const std::array<float, 12>& matrix, const Primitives::CircularTorus& params) {
    auto sides = RVMMeshHelper2::infoCircularTorusNumSides(params, m_maxSideSize, m_minSides);
    writeMesh(RVMMeshHelper2::makeCircularTorus(params, sides.first, sides.second), matrix);
}

void IFCConverter::createEllipticalDish(const std::array<float, 12>& matrix, const Primitives::EllipticalDish& params) {
    auto sides = RVMMeshHelper2::infoEllipticalDishNumSides(params, m_maxSideSize, m_minSides);
    writeMesh(RVMMeshHelper2::makeEllipticalDish(params,sides.first, sides.second), matrix);
}

void IFCConverter::createSphericalDish(const std::array<float, 12>& matrix, const Primitives::SphericalDish& params) {
    writeMesh(RVMMeshHelper2::makeSphericalDish(params, m_maxSideSize, m_minSides), matrix);
}

void IFCConverter::createSnout(const std::array<float, 12>& matrix, const Primitives::Snout& params) {
    auto sides = RVMMeshHelper2::infoSnoutNumSides(params, m_maxSideSize, m_minSides);
    writeMesh(RVMMeshHelper2::makeSnout(params, sides), matrix);
}

void IFCConverter::createCylinder(const std::array<float, 12>& matrix, const Primitives::Cylinder& params) {
    auto sides = RVMMeshHelper2::infoCylinderNumSides(params, m_maxSideSize, m_minSides);
    writeMesh(RVMMeshHelper2::makeCylinder(params, sides), matrix);
}

void IFCConverter::createSphere(const std::array<float, 12>& matrix, const Primitives::Sphere& params) {
    writeMesh( RVMMeshHelper2::makeSphere(params, m_maxSideSize, m_minSides), matrix);
}

void IFCConverter::createLine(const std::array<float, 12>& matrix, const float& startx, const float& endx) {
}

void IFCConverter::createFacetGroup(const std::array<float, 12>& m, const FGroup& vertices) {
    Eigen::Matrix4f matrix = toEigenMatrix(m);
    
    shared_ptr<IfcConnectedFaceSet> cfs (new IfcConnectedFaceSet() );
    insertEntity(cfs);

    shared_ptr<IfcFaceBasedSurfaceModel> surfaceModel ( new IfcFaceBasedSurfaceModel() );
    insertEntity(surfaceModel);
    surfaceModel->m_FbsmFaces.push_back(cfs);

    for (unsigned int i = 0; i < vertices.size(); i++) {
        shared_ptr<IfcFace> face (new IfcFace() );
        insertEntity(face);

        for (unsigned int j = 0; j < vertices[i].size(); j++) {
            shared_ptr<IfcPolyLoop> polygon (new IfcPolyLoop() );
            insertEntity(polygon);

            shared_ptr<IfcFaceBound> bound (new IfcFaceBound() );
            insertEntity(bound);
            bound->m_Bound = polygon;
            bound->m_Orientation = false;

            for (unsigned int k = 0; k < vertices[i][j].size(); k++) {
                // Transform vertex
                Vector3F v = vertices[i][j].at(k).first;
                Eigen::Vector4f vertex(v.x(), v.y(), v.z(), 1.0f);
                vertex = matrix * vertex;

                shared_ptr<IfcCartesianPoint> point (new IfcCartesianPoint() );
                insertEntity(point);
                point->m_Coordinates.push_back( shared_ptr<IfcLengthMeasure>(new IfcLengthMeasure(vertex.x()) ) );
                point->m_Coordinates.push_back( shared_ptr<IfcLengthMeasure>(new IfcLengthMeasure(vertex.y()) ) );
                point->m_Coordinates.push_back( shared_ptr<IfcLengthMeasure>(new IfcLengthMeasure(vertex.z()) ) );
                polygon->m_Polygon.push_back(point);
            }
            face->m_Bounds.push_back(bound);
        }
        // Add face to faceset
        cfs->m_CfsFaces.push_back(face);
    }

    addSurfaceModelToShape(surfaceModel);


}

shared_ptr<IfcOwnerHistory> IFCConverter::createOwnerHistory(const std::string &user) {
    if(!m_owner_history) {
        shared_ptr<IfcPerson> person( new IfcPerson() );
        person->m_Identification = shared_ptr<IfcIdentifier>( new IfcIdentifier( std::wstring(user.begin(), user.end() ) ));
        insertEntity(person);

        shared_ptr<IfcPersonAndOrganization> person_org( new IfcPersonAndOrganization() );
        person_org->m_ThePerson = person;
        insertEntity(person_org);

        m_owner_history = shared_ptr<IfcOwnerHistory>( new IfcOwnerHistory() );
        m_owner_history->m_OwningUser = person_org;
        insertEntity(m_owner_history);
    }
    return m_owner_history;
}


void IFCConverter::initModel() {

    // create Site
    shared_ptr<IfcSite> site( new IfcSite() );
    site->m_GlobalId = shared_ptr<IfcGloballyUniqueId>( new IfcGloballyUniqueId( CreateCompressedGuidString22() ) );
    site->m_OwnerHistory = m_owner_history;
    site->m_Name = shared_ptr<IfcLabel>( new IfcLabel( L"Site" ) );
    insertEntity(site);

    // create Building
    shared_ptr<IfcBuilding> building( new IfcBuilding() );
    building->m_GlobalId = shared_ptr<IfcGloballyUniqueId>( new IfcGloballyUniqueId( CreateCompressedGuidString22() ) );
    building->m_OwnerHistory = m_owner_history;
    building->m_Name = shared_ptr<IfcLabel>( new IfcLabel( L"Building" ) );
    insertEntity( building );

    // Relation project -> site
    shared_ptr<IfcRelAggregates> rel_aggregates_project_site( new IfcRelAggregates() );
    rel_aggregates_project_site->m_RelatingObject = m_model->getIfcProject();
    rel_aggregates_project_site->m_RelatedObjects.push_back(site);
    insertEntity(rel_aggregates_project_site);

    // Relation project -> site
    shared_ptr<IfcRelAggregates> rel_aggregates_site_building( new IfcRelAggregates() );
    rel_aggregates_site_building->m_RelatingObject = site;
    rel_aggregates_site_building->m_RelatedObjects.push_back(building);
    insertEntity(rel_aggregates_site_building);

    // Relation to child elements of building
    pushParentRelation(building);

    shared_ptr<IfcCartesianPoint> location( new IfcCartesianPoint() );
    insertEntity(location);
    location->m_Coordinates.push_back( shared_ptr<IfcLengthMeasure>(new IfcLengthMeasure(0.0) ) );
    location->m_Coordinates.push_back( shared_ptr<IfcLengthMeasure>(new IfcLengthMeasure(0.0) ) );
    location->m_Coordinates.push_back( shared_ptr<IfcLengthMeasure>(new IfcLengthMeasure(0.0) ) );

    shared_ptr<IfcAxis2Placement3D> world_coordinate_system( new IfcAxis2Placement3D() );
    insertEntity(world_coordinate_system);
    world_coordinate_system->m_Location = location;
    // world_coordinate_system->m_Axis = axis;
    // world_coordinate_system->m_RefDirection = ref_direction;

    // 3d representation context
    m_context = shared_ptr<IfcGeometricRepresentationContext>( new IfcGeometricRepresentationContext() );
    insertEntity(m_context);
    m_context->m_ContextType = shared_ptr<IfcLabel>( new IfcLabel( L"Model" ) );
    m_context->m_CoordinateSpaceDimension = shared_ptr<IfcDimensionCount>(new IfcDimensionCount( 3 ) );
    m_context->m_WorldCoordinateSystem = world_coordinate_system;

    m_model->getIfcProject()->m_RepresentationContexts.push_back(m_context);

    // IfcUnitAssignment

    // length unit [m]
    shared_ptr<IfcSIUnit> si_unit( new IfcSIUnit() );
    si_unit->m_UnitType = shared_ptr<IfcUnitEnum>( new IfcUnitEnum( IfcUnitEnum::ENUM_LENGTHUNIT ) );
    si_unit->m_Name = shared_ptr<IfcSIUnitName>( new IfcSIUnitName( IfcSIUnitName::ENUM_METRE ) );
    insertEntity(si_unit);

    // plane unit [rad]
    shared_ptr<IfcSIUnit> plane_angle_unit( new IfcSIUnit() );
    plane_angle_unit->m_UnitType = shared_ptr<IfcUnitEnum>( new IfcUnitEnum( IfcUnitEnum::ENUM_PLANEANGLEUNIT ) );
    plane_angle_unit->m_Name = shared_ptr<IfcSIUnitName>( new IfcSIUnitName( IfcSIUnitName::ENUM_RADIAN ) );
    insertEntity(plane_angle_unit);

    shared_ptr<IfcUnitAssignment> unitAssignment( new IfcUnitAssignment() );
    unitAssignment->m_Units.push_back( si_unit );
    unitAssignment->m_Units.push_back( plane_angle_unit );

    insertEntity(unitAssignment);
    m_model->getIfcProject()->m_UnitsInContext = unitAssignment;

}

void IFCConverter::pushParentRelation(shared_ptr<IfcObjectDefinition> parent) {
    shared_ptr<IfcRelAggregates> child_relations( new IfcRelAggregates() );
    child_relations->m_RelatingObject = parent;
    m_relationStack.push(child_relations);
}


void IFCConverter::writeMesh(const Mesh &mesh, const std::array<float, 12>& m) {
    shared_ptr<IfcConnectedFaceSet> cfs (new IfcConnectedFaceSet() );
    insertEntity(cfs);
    
    Eigen::Matrix4f matrix = toEigenMatrix(m);
    
    shared_ptr<IfcFaceBasedSurfaceModel> surfaceModel ( new IfcFaceBasedSurfaceModel() );
    insertEntity(surfaceModel);
    surfaceModel->m_FbsmFaces.push_back(cfs);

    // For each triangle
    for (unsigned int i = 0; i < mesh.positionIndex.size() / 3; i++) {
        shared_ptr<IfcFace> face (new IfcFace() );
        insertEntity(face);
        // Add face to faceset
        cfs->m_CfsFaces.push_back(face);

        shared_ptr<IfcFaceBound> bound (new IfcFaceBound() );
        insertEntity(bound);
        bound->m_Orientation = false;
        face->m_Bounds.push_back(bound);

        shared_ptr<IfcPolyLoop> polygon (new IfcPolyLoop() );
        insertEntity(polygon);

        for (unsigned int j = 0; j < 3; j++) {
            unsigned long idx = mesh.positionIndex.at(i*3+j);
            // Transform vertex
            Vector3F v = mesh.positions.at(idx);
            Eigen::Vector4f vertex(v.x(), v.y(), v.z(), 1.0f);
            vertex = matrix * vertex;

            shared_ptr<IfcCartesianPoint> point (new IfcCartesianPoint() );
            insertEntity(point);
            point->m_Coordinates.push_back( shared_ptr<IfcLengthMeasure>(new IfcLengthMeasure(vertex[0]) ) );
            point->m_Coordinates.push_back( shared_ptr<IfcLengthMeasure>(new IfcLengthMeasure(vertex[1]) ) );
            point->m_Coordinates.push_back( shared_ptr<IfcLengthMeasure>(new IfcLengthMeasure(vertex[2]) ) );
            polygon->m_Polygon.push_back(point);
        }
        bound->m_Bound = polygon;

    }
    addSurfaceModelToShape(surfaceModel);
}

void IFCConverter::addSurfaceModelToShape(shared_ptr<IfcRepresentationItem> item) {
    shared_ptr<IfcObjectDefinition> parent = m_relationStack.top()->m_RelatingObject;
    shared_ptr<IfcProduct> parentProduct = dynamic_pointer_cast<IfcProduct>(parent);
    if(parentProduct) {
        if(!parentProduct->m_Representation) {
            shared_ptr<IfcProductDefinitionShape> shape( new IfcProductDefinitionShape() );
            insertEntity(shape);

            shared_ptr<IfcShapeRepresentation> shapeRepresentation( new IfcShapeRepresentation() );
            insertEntity(shapeRepresentation);
            shapeRepresentation->m_ContextOfItems = m_context;
            shapeRepresentation->m_RepresentationIdentifier = shared_ptr<IfcLabel>( new IfcLabel( L"Building" ) );
            shapeRepresentation->m_RepresentationType = shared_ptr<IfcLabel>( new IfcLabel( L"SurfaceModel" ) );

            shape->m_Representations.push_back(shapeRepresentation);
            parentProduct->m_Representation = shape;
        }
        parentProduct->m_Representation->m_Representations[0]->m_Items.push_back(item);
    }
}

void IFCConverter::insertEntity(shared_ptr<IfcPPEntity> e) {
    if(e->m_id < 0) {
        m_currentEntityId++;
        e->m_id = m_currentEntityId;
    }
    m_model->insertEntity(e);
}

shared_ptr<IfcMaterial> IFCConverter::createMaterial(int id) {
    std::map<int, shared_ptr<IfcMaterial>>::iterator I = m_materials.find(id);
    if(I != m_materials.end()) {
        return (*I).second;
    }
    shared_ptr<IfcMaterial> material( new IfcMaterial() );
    insertEntity(material);
    material->m_Name = shared_ptr<IfcLabel>( new IfcLabel( L"Material" + std::to_wstring(id) ));
    m_materials.insert(std::make_pair(id, material));

    std::vector<float> colors = RVMColorHelper::color(id);

    shared_ptr<IfcColourRgb> surfaceColor( new IfcColourRgb() );
    insertEntity(surfaceColor);
    surfaceColor->m_Red = getNormalisedRatioMeasure(colors[0]);
    surfaceColor->m_Green = getNormalisedRatioMeasure(colors[1]);
    surfaceColor->m_Blue = getNormalisedRatioMeasure(colors[2]);

    shared_ptr<IfcSurfaceStyleRendering> surfaceStyleRendering( new IfcSurfaceStyleRendering() );
    insertEntity(surfaceStyleRendering);
    surfaceStyleRendering->m_SurfaceColour = surfaceColor;
    surfaceStyleRendering->m_Transparency = getNormalisedRatioMeasure(0.0);
    surfaceStyleRendering->m_DiffuseColour = getNormalisedRatioMeasure(1);
    surfaceStyleRendering->m_SpecularColour = getNormalisedRatioMeasure(0.25);

    shared_ptr<IfcSurfaceStyle> surfaceStyle( new IfcSurfaceStyle() );
    insertEntity(surfaceStyle);
    surfaceStyle->m_Name = shared_ptr<IfcLabel>( new IfcLabel( L"Material" + std::to_wstring(id) + L"Style"));
    surfaceStyle->m_Side = shared_ptr<IfcSurfaceSide>( new IfcSurfaceSide( IfcSurfaceSide::ENUM_BOTH ));
    surfaceStyle->m_Styles.push_back(surfaceStyleRendering);

    shared_ptr<IfcPresentationStyleAssignment> presentationStyleAssignment( new IfcPresentationStyleAssignment() );
    insertEntity(presentationStyleAssignment);
    presentationStyleAssignment->m_Styles.push_back(surfaceStyle);

    shared_ptr<IfcStyledItem> styledItem( new IfcStyledItem() );
    insertEntity(styledItem);
    styledItem->m_Styles.push_back(presentationStyleAssignment);

    shared_ptr<IfcStyledRepresentation> styledRepresentation( new IfcStyledRepresentation() );
    insertEntity(styledRepresentation);
    styledRepresentation->m_ContextOfItems = m_context;
    styledRepresentation->m_Items.push_back(styledItem);


    shared_ptr<IfcMaterialDefinitionRepresentation> materialDefinition( new IfcMaterialDefinitionRepresentation() );
    insertEntity(materialDefinition);
    materialDefinition->m_RepresentedMaterial = material;
    materialDefinition->m_Representations.push_back(styledRepresentation);

    return material;
}

void IFCConverter::messageCallBack(void* obj_ptr, shared_ptr<StatusCallback::Message> message) {
    if (message->m_message_type == StatusCallback::MESSAGE_TYPE_PROGRESS_VALUE) {
        IFCConverter* self = (IFCConverter*)obj_ptr;
        static bool first = true;
        if(first) {
            std::cout << "Writing " << self->m_filename;
            first = false;
        } else {
            std::cout << ".";
        }
    } else {
        std::wcerr << message->m_message_text << std::endl;
    }
}
