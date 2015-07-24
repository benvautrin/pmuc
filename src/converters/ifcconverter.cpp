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

#include <ifcpp/model/IfcPPGuid.h>

#include "../api/rvmmeshhelper.h"

#include <fstream>
#include <string>
#include <cassert>

IFCConverter::IFCConverter(const std::string& filename) : RVMReader(), m_filename(filename), m_model(new IfcPPModel()) {
    m_model->initFileHeader(filename);
}

IFCConverter::~IFCConverter() {
}

void IFCConverter::startDocument() {
    shared_ptr<IfcProject> project(new IfcProject( 1 ));
    project->m_GlobalId = shared_ptr<IfcGloballyUniqueId>(new IfcGloballyUniqueId( CreateCompressedGuidString22() ) );
    m_model->setIfcProject(project);
    m_model->insertEntity(project);

}

void IFCConverter::endDocument() {
    shared_ptr<IfcPPWriterSTEP> writer( new IfcPPWriterSTEP() );
    std::stringstream stream;
    std::ofstream fileStream(m_filename);

    writer->writeModelToStream(stream, m_model);
    fileStream << stream.rdbuf();
    fileStream.close();
    //std::cout << stream.str() << std::endl;
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
        m_model->insertEntity(aggregates);
    }
    m_relationStack.pop();
}

void IFCConverter::startGroup(const std::string& name, const Vector3F& translation, const int& materialId) {
    shared_ptr<IfcBuildingElementProxy> buildingElement( new IfcBuildingElementProxy() );
    m_model->insertEntity(buildingElement);
    buildingElement->m_GlobalId = shared_ptr<IfcGloballyUniqueId>(new IfcGloballyUniqueId( CreateCompressedGuidString22() ) );
    buildingElement->m_Name = shared_ptr<IfcLabel>(new IfcLabel(std::wstring(name.begin(), name.end())));

    // Translation:
    shared_ptr<IfcCartesianPoint> trans (new IfcCartesianPoint() );
    m_model->insertEntity(trans);
    trans->m_Coordinates.push_back( shared_ptr<IfcLengthMeasure>(new IfcLengthMeasure(translation[0]) ) );
    trans->m_Coordinates.push_back( shared_ptr<IfcLengthMeasure>(new IfcLengthMeasure(translation[1]) ) );
    trans->m_Coordinates.push_back( shared_ptr<IfcLengthMeasure>(new IfcLengthMeasure(translation[2]) ) );

    shared_ptr<IfcAxis2Placement3D> relativePlacement( new IfcAxis2Placement3D() );
    m_model->insertEntity(relativePlacement);
    relativePlacement->m_Location = trans;

    shared_ptr<IfcLocalPlacement> objectPlacement ( new IfcLocalPlacement() );
    m_model->insertEntity(objectPlacement);
    //objectPlacement->m_RelativePlacement = relativePlacement;
    if(m_placementStack.size()) {
        objectPlacement->m_PlacementRelTo = m_placementStack.top();
    }
    m_placementStack.push(objectPlacement);
    buildingElement->m_ObjectPlacement = objectPlacement;

    // Build relation with parent group
    m_relationStack.top()->m_RelatedObjects.push_back(buildingElement);

    // Make this group top of the stack
    pushParentRelation(buildingElement);
}

void IFCConverter::endGroup() {
    shared_ptr<IfcRelAggregates> aggregates = m_relationStack.top();

    // Do not add aggregates for empty groups
    if(aggregates->m_RelatedObjects.size() > 0) {
        m_model->insertEntity(aggregates);
    }
    m_relationStack.pop();
    m_placementStack.pop();
}

void IFCConverter::startMetaData() {

}

void IFCConverter::endMetaData() {

}

void IFCConverter::startMetaDataPair(const std::string &name, const std::string &value) {

}

void IFCConverter::endMetaDataPair() {
}

void IFCConverter::startPyramid(const std::vector<float>& matrix,
                                const float& xbottom,
                                const float& ybottom,
                                const float& xtop,
                                const float& ytop,
                                const float& height,
                                const float& xoffset,
                                const float& yoffset) {


}

void IFCConverter::endPyramid() {

}

void IFCConverter::startBox(const std::vector<float>& matrix,
                            const float& xlength,
                            const float& ylength,
                            const float& zlength) {


}

void IFCConverter::endBox() {

}

void IFCConverter::startRectangularTorus(const std::vector<float>& matrix,
                                         const float& rinside,
                                         const float& routside,
                                         const float& height,
                                         const float& angle) {

}

void IFCConverter::endRectangularTorus() {

}

void IFCConverter::startCircularTorus(const std::vector<float>& matrix,
                                      const float& rinside,
                                      const float& routside,
                                      const float& angle) {

}

void IFCConverter::endCircularTorus() {

}

void IFCConverter::startEllipticalDish(const std::vector<float>& matrix,
                                       const float& diameter,
                                       const float& radius) {
}

void IFCConverter::endEllipticalDish() {
}

void IFCConverter::startSphericalDish(const std::vector<float>& matrix,
                                      const float& diameter,
                                      const float& height) {

}

void IFCConverter::endSphericalDish() {

}

void IFCConverter::startSnout(const std::vector<float>& matrix,
                              const float& dbottom,
                              const float& dtop,
                              const float& height,
                              const float& xoffset,
                              const float& yoffset,
                              const float& unknown1,
                              const float& unknown2,
                              const float& unknown3,
                              const float& unknown4) {

}

void IFCConverter::endSnout() {
}

void IFCConverter::startCylinder(const std::vector<float>& matrix,
                                 const float& radius,
                                 const float& height) {
}

void IFCConverter::endCylinder() {
}

void IFCConverter::startSphere(const std::vector<float>& matrix,
                               const float& diameter) {
}

void IFCConverter::endSphere() {
}

void IFCConverter::startLine(const std::vector<float>& matrix,
                             const float& startx,
                             const float& endx) {
}

void IFCConverter::endLine() {
}



void IFCConverter::startFacetGroup(const std::vector<float>& matrix, const FGroup& vertices) {
    
    shared_ptr<IfcProductDefinitionShape> shape( new IfcProductDefinitionShape() );
    m_model->insertEntity(shape);

    shared_ptr<IfcConnectedFaceSet> cfs (new IfcConnectedFaceSet() );
    m_model->insertEntity(cfs);

    shared_ptr<IfcFaceBasedSurfaceModel> fbsm ( new IfcFaceBasedSurfaceModel() );
    m_model->insertEntity(fbsm);
    fbsm->m_FbsmFaces.push_back(cfs);

    unsigned long tessIndex = 0;
    for (unsigned int i = 0; i < vertices.size(); i++) {
        shared_ptr<IfcFace> face (new IfcFace() );
        m_model->insertEntity(face);

        for (unsigned int j = 0; j < vertices[i].size(); j++) {
            shared_ptr<IfcPolyLoop> polygon (new IfcPolyLoop() );
            m_model->insertEntity(polygon);

            shared_ptr<IfcFaceOuterBound> bound (new IfcFaceOuterBound() );
            m_model->insertEntity(bound);
            bound->m_Bound = polygon;
            bound->m_Orientation = true;

            for (unsigned int k = 0; k < vertices[i][j].size(); k++) {
                Vector3F vertex(vertices[i][j].at(k).first);
                shared_ptr<IfcCartesianPoint> point (new IfcCartesianPoint() );
                m_model->insertEntity(point);
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

    shared_ptr<IfcShapeRepresentation> shapeRepresentation( new IfcShapeRepresentation() );
    m_model->insertEntity(shapeRepresentation);
    shapeRepresentation->m_ContextOfItems = m_context;
    shapeRepresentation->m_RepresentationIdentifier = shared_ptr<IfcLabel>( new IfcLabel( L"Building" ) );
    shapeRepresentation->m_RepresentationType = shared_ptr<IfcLabel>( new IfcLabel( L"SurfaceModel" ) );
    shapeRepresentation->m_Items.push_back(fbsm);

    shape->m_Representations.push_back(shapeRepresentation);


    shared_ptr<IfcObjectDefinition> parent = m_relationStack.top()->m_RelatingObject;
    shared_ptr<IfcProduct> parentProduct = dynamic_pointer_cast<IfcProduct>(parent);
    if(parentProduct) {
        parentProduct->m_Representation = shape;
    }
}

void IFCConverter::endFacetGroup() {
}

shared_ptr<IfcOwnerHistory> IFCConverter::createOwnerHistory(const std::string &user) {
    if(!m_owner_history) {
        shared_ptr<IfcPerson> person( new IfcPerson() );
        person->m_Identification = shared_ptr<IfcIdentifier>( new IfcIdentifier( std::wstring(user.begin(), user.end() ) ));
        m_model->insertEntity(person);

        shared_ptr<IfcPersonAndOrganization> person_org( new IfcPersonAndOrganization() );
        person_org->m_ThePerson = person;
        m_model->insertEntity(person_org);

        m_owner_history = shared_ptr<IfcOwnerHistory>( new IfcOwnerHistory() );
        m_owner_history->m_OwningUser = person_org;
        m_model->insertEntity(m_owner_history);
    }
    return m_owner_history;
}


void IFCConverter::initModel() {

    // create Site
    shared_ptr<IfcSite> site( new IfcSite() );
    site->m_GlobalId = shared_ptr<IfcGloballyUniqueId>( new IfcGloballyUniqueId( CreateCompressedGuidString22() ) );
    site->m_OwnerHistory = m_owner_history;
    site->m_Name = shared_ptr<IfcLabel>( new IfcLabel( L"Site" ) );
    m_model->insertEntity(site);

    // create Building
    shared_ptr<IfcBuilding> building( new IfcBuilding() );
    building->m_GlobalId = shared_ptr<IfcGloballyUniqueId>( new IfcGloballyUniqueId( CreateCompressedGuidString22() ) );
    building->m_OwnerHistory = m_owner_history;
    building->m_Name = shared_ptr<IfcLabel>( new IfcLabel( L"Building" ) );
    m_model->insertEntity( building );

    // Relation project -> site
    shared_ptr<IfcRelAggregates> rel_aggregates_project_site( new IfcRelAggregates() );
    rel_aggregates_project_site->m_RelatingObject = m_model->getIfcProject();
    rel_aggregates_project_site->m_RelatedObjects.push_back(site);
    m_model->insertEntity(rel_aggregates_project_site);

    // Relation project -> site
    shared_ptr<IfcRelAggregates> rel_aggregates_site_building( new IfcRelAggregates() );
    rel_aggregates_site_building->m_RelatingObject = site;
    rel_aggregates_site_building->m_RelatedObjects.push_back(building);
    m_model->insertEntity(rel_aggregates_site_building);

    // Relation to child elements of building
    pushParentRelation(building);

    shared_ptr<IfcCartesianPoint> location( new IfcCartesianPoint() );
    m_model->insertEntity(location);
    location->m_Coordinates.push_back( shared_ptr<IfcLengthMeasure>(new IfcLengthMeasure(0.0) ) );
    location->m_Coordinates.push_back( shared_ptr<IfcLengthMeasure>(new IfcLengthMeasure(0.0) ) );
    location->m_Coordinates.push_back( shared_ptr<IfcLengthMeasure>(new IfcLengthMeasure(0.0) ) );

    shared_ptr<IfcAxis2Placement3D> world_coordinate_system( new IfcAxis2Placement3D() );
    m_model->insertEntity(world_coordinate_system);
    world_coordinate_system->m_Location = location;
    // world_coordinate_system->m_Axis = axis;
    // world_coordinate_system->m_RefDirection = ref_direction;

    // 3d representation context
    m_context = shared_ptr<IfcGeometricRepresentationContext>( new IfcGeometricRepresentationContext() );
    m_model->insertEntity(m_context);
    m_context->m_ContextType = shared_ptr<IfcLabel>( new IfcLabel( L"Model" ) );
    m_context->m_CoordinateSpaceDimension = shared_ptr<IfcDimensionCount>(new IfcDimensionCount( 3 ) );
    m_context->m_WorldCoordinateSystem = world_coordinate_system;

    m_model->getIfcProject()->m_RepresentationContexts.push_back(m_context);

    // IfcUnitAssignment

    // length unit [m]
    shared_ptr<IfcSIUnit> si_unit( new IfcSIUnit() );
    si_unit->m_UnitType = shared_ptr<IfcUnitEnum>( new IfcUnitEnum( IfcUnitEnum::ENUM_LENGTHUNIT ) );
    si_unit->m_Name = shared_ptr<IfcSIUnitName>( new IfcSIUnitName( IfcSIUnitName::ENUM_METRE ) );
    m_model->insertEntity(si_unit);

    // plane unit [rad]
    shared_ptr<IfcSIUnit> plane_angle_unit( new IfcSIUnit() );
    plane_angle_unit->m_UnitType = shared_ptr<IfcUnitEnum>( new IfcUnitEnum( IfcUnitEnum::ENUM_PLANEANGLEUNIT ) );
    plane_angle_unit->m_Name = shared_ptr<IfcSIUnitName>( new IfcSIUnitName( IfcSIUnitName::ENUM_RADIAN ) );
    m_model->insertEntity(plane_angle_unit);

    shared_ptr<IfcUnitAssignment> unitAssignment( new IfcUnitAssignment() );
    unitAssignment->m_Units.push_back( si_unit );
    unitAssignment->m_Units.push_back( plane_angle_unit );

    m_model->insertEntity(unitAssignment);
    m_model->getIfcProject()->m_UnitsInContext = unitAssignment;

}

void IFCConverter::pushParentRelation(shared_ptr<IfcObjectDefinition> parent) {
    shared_ptr<IfcRelAggregates> child_relations( new IfcRelAggregates() );
    child_relations->m_RelatingObject = parent;
    m_relationStack.push(child_relations);
}
