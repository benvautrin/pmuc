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

#ifdef _MSC_VER
#define _USE_MATH_DEFINES // For PI under VC++
#endif

#include <cmath>

#define BOOST_DATE_TIME_NO_LIB
#include <boost/algorithm/string/replace.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/posix_time/posix_time_io.hpp>
#include <boost/locale/encoding_utf.hpp>

#include <Eigen/Core>
#include <Eigen/Geometry>
#include <Eigen/SVD>


#include "../api/rvmmeshhelper.h"
#include "../api/rvmcolorhelper.h"

#include <fstream>
#include <string>
#include <cassert>

using boost::locale::conv::utf_to_utf;

namespace {
   /* shared_ptr<IfcNormalisedRatioMeasure> getNormalisedRatioMeasure(double value) {
        shared_ptr<IfcNormalisedRatioMeasure> measure( new IfcNormalisedRatioMeasure() );
        measure->m_value = value;
        return measure;
    }*/

    std::wstring utf8_to_wstring(const std::string& str)
    {
        return utf_to_utf<wchar_t>(str.c_str(), str.c_str() + str.size());
    }

    Transform3f toEigenTransform(const std::array<float, 12>& matrix) {
        Transform3f result;
        result.setIdentity();
        for (unsigned int i = 0; i < 3; i++) {
            for (unsigned int j = 0; j < 4; j++) {
                result(i, j) = matrix[i+j*3];
            }
        }
        return result;
    }

    Eigen::Matrix4f toEigenMatrix(const std::array<float, 12>& matrix) {
        return toEigenTransform(matrix).matrix();
    }

    int toTimeStamp(const std::string& schema) {
        // TODO: Implement
        return 0;
    }

    float getScaleFromTransformation(const Transform3f& transform) {
        Eigen::Matrix3f rotation;
        Eigen::Matrix3f scale;
        transform.computeRotationScaling(&rotation, &scale);
        return scale(0,0);
    }
}

IFCConverter::IFCConverter(const std::string& filename, const std::string& schema)
    : RVMReader(), m_filename(filename), m_currentEntityId(0) {
  m_writer = new IFCStreamWriter(filename);
  m_writer->startDocument();

  FileDescription desc;
  desc.description.push_back("PMUC generated IFC file.");

  // 2011-04-21T14:25:12
  std::locale loc(std::wcout.getloc(), new boost::posix_time::wtime_facet(L"%Y-%m-%dT%H:%M:%S"));
  std::stringstream wss;
  wss.imbue(loc);
  boost::posix_time::ptime now = boost::posix_time::second_clock::local_time();
  wss << now;
  std::string ts = wss.str();

  FileName name;
  name.name = boost::replace_all_copy(filename, "\\", "\\\\");
  name.time_stamp_text = ts;
  name.preprocessor_version = "IfcPlusPlus";

  m_writer->addHeader(desc, name);
}

IFCConverter::~IFCConverter() {
}

void IFCConverter::startDocument() {

}

void IFCConverter::endDocument() {
    m_writer->endDocument();
}

void IFCConverter::startHeader(const std::string& banner, const std::string& fileNote, const std::string& dateStr, const std::string& user, const std::string& encoding) {
    m_project = new IfcEntity("IFCPROJECT");
    auto ownerHistory = createOwnerHistory(user, banner, toTimeStamp(dateStr));
    m_project->attributes = { createBase64Uuid<char>(), ownerHistory };

}

void IFCConverter::endHeader() {

}

void IFCConverter::startModel(const std::string& projectName, const std::string& name) {
    // TODO(ksons): Which is long name and which is name
    /*m_model->getIfcProject()->m_Name = shared_ptr<IfcLabel>(new IfcLabel( utf8_to_wstring( projectName )));
    m_model->getIfcProject()->m_LongName = shared_ptr<IfcLabel>(new IfcLabel( utf8_to_wstring( name )));
    m_model->getIfcProject()->m_Phase = shared_ptr<IfcLabel>(new IfcLabel(L"$"));*/
    m_project->attributes.push_back(projectName); // name
    m_project->attributes.push_back(name); // description
    m_project->attributes.push_back(IFC_STRING_UNSET); // object type
    m_project->attributes.push_back(IFC_STRING_UNSET); // long name
    m_project->attributes.push_back(IFC_STRING_UNSET); // phase

    m_writer->addEntity(*m_project);



    // Initialize model with site, building etc
    initModel();


}

void IFCConverter::endModel() {
    /*assert(m_relationStack.size() == 1);
    shared_ptr<IfcRelAggregates> aggregates = m_relationStack.top();

    // Do not add aggregates for empty model
    if(aggregates->m_RelatedObjects.size() > 0) {
        insertEntity(aggregates);
    }
    m_relationStack.pop();*/
}

void IFCConverter::startGroup(const std::string& name, const Vector3F& translation, const int& materialId) {
    /*shared_ptr<IfcBuildingElementProxy> buildingElement( new IfcBuildingElementProxy() );
    buildingElement->m_GlobalId = shared_ptr<IfcGloballyUniqueId>(new IfcGloballyUniqueId( CreateCompressedGuidString22() ) );
    buildingElement->m_Name = shared_ptr<IfcLabel>(new IfcLabel( utf8_to_wstring( name ) ) );
    buildingElement->m_OwnerHistory = m_owner_history;
    insertEntity(buildingElement);

    shared_ptr<IfcMaterial> material = createMaterial(materialId);
    shared_ptr<IfcRelAssociatesMaterial> materialAssociates( new IfcRelAssociatesMaterial() );
    materialAssociates->m_GlobalId = shared_ptr<IfcGloballyUniqueId>(new IfcGloballyUniqueId( CreateCompressedGuidString22() ) );
    materialAssociates->m_OwnerHistory = m_owner_history;
    materialAssociates->m_RelatedObjects.push_back(buildingElement);
    materialAssociates->m_RelatingMaterial = material;
    insertEntity(materialAssociates);

    m_currentMaterial.push(materialId);

    // Build relation with parent group
    m_relationStack.top()->m_RelatedObjects.push_back(buildingElement);

    // Make this group top of the stack
    pushParentRelation(buildingElement);*/
}

void IFCConverter::endGroup() {
    /*shared_ptr<IfcRelAggregates> aggregates = m_relationStack.top();

    // Do not add aggregates for empty groups
    if(aggregates->m_RelatedObjects.size() > 0) {
        insertEntity(aggregates);
    }
    m_relationStack.pop();
    m_currentMaterial.pop();*/
}

void IFCConverter::startMetaData() {
    // Create a property set that holds all upcoming properties
    /*m_propertySet = shared_ptr<IfcPropertySet>( new IfcPropertySet() );
    m_propertySet->m_GlobalId = shared_ptr<IfcGloballyUniqueId>(new IfcGloballyUniqueId( CreateCompressedGuidString22() ) );
    m_propertySet->m_OwnerHistory = m_owner_history;
    m_propertySet->m_Name = shared_ptr<IfcLabel>( new IfcLabel( L"RVM Attributes" ));
    m_propertySet->m_Description = shared_ptr<IfcText>( new IfcText( L"Attributes from RVM Attribute file" ));
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
    insertEntity(propertyRelation);*/

}

void IFCConverter::endMetaData() {

}

void IFCConverter::startMetaDataPair(const std::string &name, const std::string &value) {
    /*assert(m_propertySet);
    std::string value_escaped = boost::replace_all_copy(value, "\\", "\\\\");

    shared_ptr<IfcPropertySingleValue> prop( new IfcPropertySingleValue() );
    insertEntity(prop);
    prop->m_Name = shared_ptr<IfcIdentifier>( new IfcIdentifier( utf8_to_wstring(name) ));
    prop->m_NominalValue = shared_ptr<IfcLabel>(new IfcLabel( utf8_to_wstring(value_escaped) ));

    m_propertySet->m_HasProperties.push_back(prop);*/
}

void IFCConverter::endMetaDataPair() {
}

void IFCConverter::createPyramid(const std::array<float, 12>& matrix, const Primitives::Pyramid& params) {
    writeMesh(RVMMeshHelper2::makePyramid(params, m_maxSideSize, m_minSides), matrix);
}


void IFCConverter::createBox(const std::array<float, 12>& matrix, const Primitives::Box& params) {
    /*if (m_primitives) {
        Transform3f transform = toEigenTransform(matrix);
        float s = getScaleFromTransformation(transform);

        shared_ptr<IfcPositiveLengthMeasure> xDim (new IfcPositiveLengthMeasure() );
        xDim->m_value = params.len[0] *  s;

        shared_ptr<IfcPositiveLengthMeasure> yDim (new IfcPositiveLengthMeasure() );
        yDim->m_value = params.len[1] *  s;

        shared_ptr<IfcPositiveLengthMeasure> zDim (new IfcPositiveLengthMeasure() );
        zDim->m_value = params.len[2] * s;

        shared_ptr<IfcCartesianPoint> location( new IfcCartesianPoint() );
        insertEntity(location);
        location->m_Coordinates.push_back( shared_ptr<IfcLengthMeasure>(new IfcLengthMeasure(0.0) ) );
        location->m_Coordinates.push_back( shared_ptr<IfcLengthMeasure>(new IfcLengthMeasure(0.0) ) );

        shared_ptr<IfcAxis2Placement2D> position (new IfcAxis2Placement2D() );
        insertEntity(position);
        position->m_Location = location;

        shared_ptr<IfcRectangleProfileDef> profile (new IfcRectangleProfileDef() );
        insertEntity(profile);
        profile->m_ProfileType = shared_ptr<IfcProfileTypeEnum>( new IfcProfileTypeEnum( IfcProfileTypeEnum::ENUM_AREA ) );
        profile->m_Position = position;
        profile->m_XDim = xDim;
        profile->m_YDim = yDim;

        shared_ptr<IfcDirection> direction (new IfcDirection() );
        insertEntity(direction);
        direction->m_DirectionRatios.push_back(0);
        direction->m_DirectionRatios.push_back(0);
        direction->m_DirectionRatios.push_back(1);

        Eigen::Vector3f offset(0, 0, -params.len[2] * 0.5f *  s);

        shared_ptr<IfcExtrudedAreaSolid> box (new IfcExtrudedAreaSolid() );
        insertEntity(box);
        box->m_Depth = zDim;
        box->m_Position = getCoordinateSystem(toEigenTransform(matrix), offset);
        box->m_SweptArea = profile;
        box->m_ExtrudedDirection = direction;

        addRepresentationToShape(box, shared_ptr<IfcLabel>( new IfcLabel( L"SweptSolid" ) ));
    } else {
        writeMesh(RVMMeshHelper2::makeBox(params, m_maxSideSize, m_minSides), matrix);
    }
	*/
}

void IFCConverter::createRectangularTorus(const std::array<float, 12>& matrix, const Primitives::RectangularTorus& params) {
    /*if(m_primitives) {
        Transform3f transform = toEigenTransform(matrix);
        const float s = getScaleFromTransformation(transform);

        // Define the parametric profile first
        shared_ptr<IfcPositiveLengthMeasure> xDim (new IfcPositiveLengthMeasure() );
        xDim->m_value = params.height() * s;

        float yExtend = (params.routside() - params.rinside()) * s;
        shared_ptr<IfcPositiveLengthMeasure> yDim (new IfcPositiveLengthMeasure() );
        yDim->m_value = yExtend;

        shared_ptr<IfcCartesianPoint> location( new IfcCartesianPoint() );
        insertEntity(location);
        location->m_Coordinates.push_back( shared_ptr<IfcLengthMeasure>(new IfcLengthMeasure(0.0) ) );
        location->m_Coordinates.push_back( shared_ptr<IfcLengthMeasure>(new IfcLengthMeasure(params.rinside() * s + 0.5 * yExtend) ) );

        shared_ptr<IfcAxis2Placement2D> position (new IfcAxis2Placement2D() );
        insertEntity(position);
        position->m_Location = location;

        shared_ptr<IfcRectangleProfileDef> profile (new IfcRectangleProfileDef() );
        insertEntity(profile);
        profile->m_ProfileType = shared_ptr<IfcProfileTypeEnum>( new IfcProfileTypeEnum( IfcProfileTypeEnum::ENUM_AREA ) );
        profile->m_Position = position;
        profile->m_XDim = xDim;
        profile->m_YDim = yDim;

        shared_ptr<IfcDirection> axis (new IfcDirection() );
        insertEntity(axis);
        axis->m_DirectionRatios.push_back(1);
        axis->m_DirectionRatios.push_back(0);
        axis->m_DirectionRatios.push_back(0);

        addRevolvedAreaSolidToShape(profile, axis, params.angle(), transform.rotate(Eigen::AngleAxisf(float(0.5*M_PI),  Eigen::Vector3f::UnitY())));
    } else {
        writeMesh(RVMMeshHelper2::makeRectangularTorus(params, m_maxSideSize, m_minSides), matrix);
    }*/
}

void IFCConverter::createCircularTorus(const std::array<float, 12>& matrix, const Primitives::CircularTorus& params) {
    /*if(m_primitives) {
        Transform3f transform = toEigenTransform(matrix);
        const float s = getScaleFromTransformation(transform);

        // Define the parametric profile first
        shared_ptr<IfcPositiveLengthMeasure> radius (new IfcPositiveLengthMeasure() );
        radius->m_value = params.radius() * s;

        shared_ptr<IfcCartesianPoint> profile_location( new IfcCartesianPoint() );
        insertEntity(profile_location);
        profile_location->m_Coordinates.push_back( shared_ptr<IfcLengthMeasure>( new IfcLengthMeasure(0.0) ) );
        profile_location->m_Coordinates.push_back( shared_ptr<IfcLengthMeasure>( new IfcLengthMeasure(params.offset() * s) ) );

        shared_ptr<IfcAxis2Placement2D> position (new IfcAxis2Placement2D() );
        insertEntity(position);
        position->m_Location = profile_location;

        shared_ptr<IfcCircleProfileDef> profile (new IfcCircleProfileDef() );
        insertEntity(profile);
        profile->m_ProfileType = shared_ptr<IfcProfileTypeEnum>( new IfcProfileTypeEnum( IfcProfileTypeEnum::ENUM_AREA ) );
        profile->m_Position = position;
        profile->m_Radius = radius;

        shared_ptr<IfcDirection> axis (new IfcDirection() );
        insertEntity(axis);
        axis->m_DirectionRatios.push_back(1);
        axis->m_DirectionRatios.push_back(0);
        axis->m_DirectionRatios.push_back(0);

        addRevolvedAreaSolidToShape(profile, axis, params.angle(), transform.rotate(Eigen::AngleAxisf(float(0.5*M_PI),  Eigen::Vector3f::UnitY())));
    } else {
        auto sides = RVMMeshHelper2::infoCircularTorusNumSides(params, m_maxSideSize, m_minSides);
        writeMesh(RVMMeshHelper2::makeCircularTorus(params, sides.first, sides.second), matrix);
    }*/
}

void IFCConverter::createEllipticalDish(const std::array<float, 12>& matrix, const Primitives::EllipticalDish& params) {
    /*if(m_primitives) {
        Transform3f transform = toEigenTransform(matrix);
        const float s = getScaleFromTransformation(transform);

        double r = params.diameter() * s;
        double r2 = params.radius() * s;

        shared_ptr<IfcPositiveLengthMeasure> radius (new IfcPositiveLengthMeasure() );
        radius->m_value = r;

        shared_ptr<IfcPositiveLengthMeasure> radius2 (new IfcPositiveLengthMeasure() );
        radius2->m_value = r2;

        shared_ptr<IfcCartesianPoint> location( new IfcCartesianPoint() );
        insertEntity(location);
        location->m_Coordinates.push_back( shared_ptr<IfcLengthMeasure>(new IfcLengthMeasure(0.0) ) );
        location->m_Coordinates.push_back( shared_ptr<IfcLengthMeasure>(new IfcLengthMeasure(0.0) ) );

        shared_ptr<IfcDirection> direction( new IfcDirection() );
        insertEntity(direction);
        direction->m_DirectionRatios.push_back(0);
        direction->m_DirectionRatios.push_back(1);

        shared_ptr<IfcAxis2Placement2D> position( new IfcAxis2Placement2D() );
        insertEntity(position);
        position->m_Location = location;
        position->m_RefDirection = direction;

        shared_ptr<IfcEllipse> ellipse (new IfcEllipse() );
        insertEntity(ellipse);
        ellipse->m_SemiAxis1 = radius2;
        ellipse->m_SemiAxis2 = radius;
        ellipse->m_Position = position;

        shared_ptr<IfcCartesianPoint> p1 (new IfcCartesianPoint() );
        insertEntity(p1);
        p1->m_Coordinates.push_back( shared_ptr<IfcLengthMeasure>(new IfcLengthMeasure( r ) ) );
        p1->m_Coordinates.push_back( shared_ptr<IfcLengthMeasure>(new IfcLengthMeasure( 0.0 ) ) );

        shared_ptr<IfcCartesianPoint> p2 (new IfcCartesianPoint() );
        insertEntity(p2);
        p2->m_Coordinates.push_back( shared_ptr<IfcLengthMeasure>(new IfcLengthMeasure( 0.0 ) ) );
        p2->m_Coordinates.push_back( shared_ptr<IfcLengthMeasure>(new IfcLengthMeasure( 0.0 ) ) );

        shared_ptr<IfcCartesianPoint> p3 (new IfcCartesianPoint() );
        insertEntity(p3);
        p3->m_Coordinates.push_back( shared_ptr<IfcLengthMeasure>(new IfcLengthMeasure( 0.0 ) ) );
        p3->m_Coordinates.push_back( shared_ptr<IfcLengthMeasure>(new IfcLengthMeasure( r2 ) ) );

        shared_ptr<IfcPolyline> line (new IfcPolyline() );
        insertEntity(line);
        line->m_Points.push_back(p1);
        line->m_Points.push_back(p2);
        line->m_Points.push_back(p3);

        shared_ptr<IfcCompositeCurveSegment> segLine (new IfcCompositeCurveSegment() );
        insertEntity(segLine);
        segLine->m_Transition = shared_ptr<IfcTransitionCode>(new IfcTransitionCode( IfcTransitionCode::ENUM_CONTINUOUS ));
        segLine->m_SameSense = true;
        segLine->m_ParentCurve = line;

        shared_ptr<IfcTrimmedCurve> curve (new IfcTrimmedCurve() );
        insertEntity(curve);
        curve->m_BasisCurve = ellipse;
        curve->m_Trim1.push_back(p3);
        curve->m_Trim2.push_back(p1);
        curve->m_SenseAgreement = false;
        curve->m_MasterRepresentation = shared_ptr<IfcTrimmingPreference>(new IfcTrimmingPreference( IfcTrimmingPreference::ENUM_CARTESIAN ) );

        shared_ptr<IfcCompositeCurveSegment> segCurve (new IfcCompositeCurveSegment() );
        insertEntity(segCurve);
        segCurve->m_Transition = shared_ptr<IfcTransitionCode>(new IfcTransitionCode( IfcTransitionCode::ENUM_CONTINUOUS ));
        segCurve->m_SameSense = true;
        segCurve->m_ParentCurve = curve;

        shared_ptr<IfcCompositeCurve> compositeCurve (new IfcCompositeCurve() );
        insertEntity(compositeCurve);
        compositeCurve->m_Segments.push_back(segLine);
        compositeCurve->m_Segments.push_back(segCurve);
        compositeCurve->m_SelfIntersect = LogicalEnum::LOGICAL_FALSE;

        shared_ptr<IfcArbitraryClosedProfileDef> profile( new IfcArbitraryClosedProfileDef() );
        insertEntity(profile);
        profile->m_ProfileType = shared_ptr<IfcProfileTypeEnum>( new IfcProfileTypeEnum( IfcProfileTypeEnum::ENUM_AREA ) );
        profile->m_OuterCurve = compositeCurve;

        shared_ptr<IfcDirection> axis (new IfcDirection() );
        insertEntity(axis);
        axis->m_DirectionRatios.push_back(0);
        axis->m_DirectionRatios.push_back(1);
        axis->m_DirectionRatios.push_back(0);

        addRevolvedAreaSolidToShape(profile, axis, 2.0 * M_PI, transform.rotate(Eigen::AngleAxisf(float(0.5*M_PI),  Eigen::Vector3f::UnitX())));
    } else {
        auto sides = RVMMeshHelper2::infoEllipticalDishNumSides(params, m_maxSideSize, m_minSides);
        writeMesh(RVMMeshHelper2::makeEllipticalDish(params,sides.first, sides.second), matrix);
    }*/
}

void IFCConverter::createSphericalDish(const std::array<float, 12>& matrix, const Primitives::SphericalDish& params) {
    /*if(m_primitives) {
        Transform3f transform = toEigenTransform(matrix);
        const float s = getScaleFromTransformation(transform);

        double r = params.diameter() * 0.5 * s;
        double h = params.height() * s;
        double offset = r - h;
        double angle =  asin(1.0 - h / r);

        shared_ptr<IfcPositiveLengthMeasure> radius (new IfcPositiveLengthMeasure() );
        radius->m_value = r;

        shared_ptr<IfcCartesianPoint> location( new IfcCartesianPoint() );
        insertEntity(location);
        location->m_Coordinates.push_back( shared_ptr<IfcLengthMeasure>(new IfcLengthMeasure(0.0) ) );
        location->m_Coordinates.push_back( shared_ptr<IfcLengthMeasure>(new IfcLengthMeasure(-offset) ) );

        shared_ptr<IfcDirection> direction( new IfcDirection() );
        insertEntity(direction);
        direction->m_DirectionRatios.push_back(0);
        direction->m_DirectionRatios.push_back(1);

        shared_ptr<IfcAxis2Placement2D> position( new IfcAxis2Placement2D() );
        insertEntity(position);
        position->m_Location = location;
        position->m_RefDirection = direction;

        shared_ptr<IfcCircle> circle (new IfcCircle() );
        insertEntity(circle);
        circle->m_Radius = radius;
        circle->m_Position = position;

        shared_ptr<IfcCartesianPoint> p1 (new IfcCartesianPoint() );
        insertEntity(p1);
        p1->m_Coordinates.push_back( shared_ptr<IfcLengthMeasure>(new IfcLengthMeasure( r * cos(angle) ) ) );
        p1->m_Coordinates.push_back( shared_ptr<IfcLengthMeasure>(new IfcLengthMeasure( r * sin(angle) - offset ) ) );

        shared_ptr<IfcCartesianPoint> p2 (new IfcCartesianPoint() );
        insertEntity(p2);
        p2->m_Coordinates.push_back( shared_ptr<IfcLengthMeasure>(new IfcLengthMeasure( 0.0 ) ) );
        p2->m_Coordinates.push_back( shared_ptr<IfcLengthMeasure>(new IfcLengthMeasure( 0.0 ) ) );

        shared_ptr<IfcCartesianPoint> p3 (new IfcCartesianPoint() );
        insertEntity(p3);
        p3->m_Coordinates.push_back( shared_ptr<IfcLengthMeasure>(new IfcLengthMeasure( 0.0 ) ) );
        p3->m_Coordinates.push_back( shared_ptr<IfcLengthMeasure>(new IfcLengthMeasure( h ) ) );

        shared_ptr<IfcPolyline> line (new IfcPolyline() );
        insertEntity(line);
        line->m_Points.push_back(p1);
        line->m_Points.push_back(p2);
        line->m_Points.push_back(p3);

        shared_ptr<IfcCompositeCurveSegment> segLine (new IfcCompositeCurveSegment() );
        insertEntity(segLine);
        segLine->m_Transition = shared_ptr<IfcTransitionCode>(new IfcTransitionCode( IfcTransitionCode::ENUM_CONTINUOUS ));
        segLine->m_SameSense = true;
        segLine->m_ParentCurve = line;

        shared_ptr<IfcTrimmedCurve> curve (new IfcTrimmedCurve() );
        insertEntity(curve);
        curve->m_BasisCurve = circle;
        curve->m_Trim1.push_back(p3);
        curve->m_Trim2.push_back(p1);
        curve->m_SenseAgreement = false;
        curve->m_MasterRepresentation = shared_ptr<IfcTrimmingPreference>(new IfcTrimmingPreference( IfcTrimmingPreference::ENUM_CARTESIAN ) );

        shared_ptr<IfcCompositeCurveSegment> segCurve (new IfcCompositeCurveSegment() );
        insertEntity(segCurve);
        segCurve->m_Transition = shared_ptr<IfcTransitionCode>(new IfcTransitionCode( IfcTransitionCode::ENUM_CONTINUOUS ));
        segCurve->m_SameSense = true;
        segCurve->m_ParentCurve = curve;

        shared_ptr<IfcCompositeCurve> compositeCurve (new IfcCompositeCurve() );
        insertEntity(compositeCurve);
        compositeCurve->m_Segments.push_back(segLine);
        compositeCurve->m_Segments.push_back(segCurve);
        compositeCurve->m_SelfIntersect = LogicalEnum::LOGICAL_FALSE;

        shared_ptr<IfcArbitraryClosedProfileDef> profile( new IfcArbitraryClosedProfileDef() );
        insertEntity(profile);
        profile->m_ProfileType = shared_ptr<IfcProfileTypeEnum>( new IfcProfileTypeEnum( IfcProfileTypeEnum::ENUM_AREA ) );
        profile->m_OuterCurve = compositeCurve;

        shared_ptr<IfcDirection> axis (new IfcDirection() );
        insertEntity(axis);
        axis->m_DirectionRatios.push_back(0);
        axis->m_DirectionRatios.push_back(1);
        axis->m_DirectionRatios.push_back(0);

        addRevolvedAreaSolidToShape(profile, axis, 2.0 * M_PI, transform.rotate(Eigen::AngleAxisf(float(0.5*M_PI),  Eigen::Vector3f::UnitX())));
    } else {
        writeMesh(RVMMeshHelper2::makeSphericalDish(params, m_maxSideSize, m_minSides), matrix);
    }*/
}

void IFCConverter::createSnout(const std::array<float, 12>& matrix, const Primitives::Snout& params) {
    if(params.xtshear() > FLT_EPSILON || params.ytshear() > FLT_EPSILON || params.xbshear() > FLT_EPSILON || params.ybshear() > FLT_EPSILON) {
        createSlopedCylinder(matrix, params);
    } else {
        auto sides = RVMMeshHelper2::infoSnoutNumSides(params, m_maxSideSize, m_minSides);
        writeMesh(RVMMeshHelper2::makeSnout(params, sides), matrix);
    }
}

void IFCConverter::createSlopedCylinder(const std::array<float, 12>& matrix, const Primitives::Snout& params) {
   /* if(m_primitives && std::abs(params.dtop() - params.dbottom()) < std::numeric_limits<float>::epsilon()) {
        const auto transform = toEigenTransform(matrix);
        const float s = getScaleFromTransformation(transform);

        const float r = params.dtop();
        const float halfHeight = params.height() * 0.5f;
        const float topOffset = r * std::max(std::abs(tan(params.xtshear())),std::abs(tan(params.ytshear())));
        const float bottomOffset = r * std::max(std::abs(tan(params.xtshear())),std::abs(tan(params.ytshear())));

        shared_ptr<IfcPositiveLengthMeasure> height (new IfcPositiveLengthMeasure() );
        height->m_value = (params.height() + topOffset + bottomOffset) * s;

        shared_ptr<IfcPositiveLengthMeasure> radius (new IfcPositiveLengthMeasure() );
        radius->m_value = r * s;

        shared_ptr<IfcCartesianPoint> location( new IfcCartesianPoint() );
        insertEntity(location);
        location->m_Coordinates.push_back( shared_ptr<IfcLengthMeasure>(new IfcLengthMeasure(0.0) ) );
        location->m_Coordinates.push_back( shared_ptr<IfcLengthMeasure>(new IfcLengthMeasure(0.0) ) );

        shared_ptr<IfcAxis2Placement2D> position (new IfcAxis2Placement2D() );
        insertEntity(position);
        position->m_Location = location;

        shared_ptr<IfcCircleProfileDef> profile (new IfcCircleProfileDef() );
        insertEntity(profile);
        profile->m_ProfileType = shared_ptr<IfcProfileTypeEnum>( new IfcProfileTypeEnum( IfcProfileTypeEnum::ENUM_AREA ) );
        profile->m_Position = position;
        profile->m_Radius = radius;

        shared_ptr<IfcDirection> direction (new IfcDirection() );
        insertEntity(direction);
        direction->m_DirectionRatios.push_back(0);
        direction->m_DirectionRatios.push_back(0);
        direction->m_DirectionRatios.push_back(1);

        Eigen::Vector3f offset(0, 0, -(halfHeight + bottomOffset) *  s);

        shared_ptr<IfcExtrudedAreaSolid> cylinder ( new IfcExtrudedAreaSolid() );
        insertEntity(cylinder);
        cylinder->m_Depth = height;
        cylinder->m_Position = getCoordinateSystem(transform, offset);
        cylinder->m_SweptArea = profile;
        cylinder->m_ExtrudedDirection = direction;

        shared_ptr<IfcPlane> planeTop = createClippingPlane(halfHeight * s, Eigen::Vector3d(-sin(params.xtshear())*cos(params.ytshear()), -sin(params.ytshear()), cos(params.xtshear())*cos(params.ytshear())));

        shared_ptr<IfcHalfSpaceSolid> halfSpaceSolid ( new IfcHalfSpaceSolid() );
        insertEntity(halfSpaceSolid);
        halfSpaceSolid->m_BaseSurface = planeTop;
        halfSpaceSolid->m_AgreementFlag = false;

        shared_ptr<IfcBooleanClippingResult> clippingResult1 ( new IfcBooleanClippingResult() );
        insertEntity(clippingResult1);
        clippingResult1->m_Operator = shared_ptr<IfcBooleanOperator >( new IfcBooleanOperator( IfcBooleanOperator::ENUM_DIFFERENCE ) );
        clippingResult1->m_FirstOperand = cylinder;
        clippingResult1->m_SecondOperand = halfSpaceSolid;

        shared_ptr<IfcPlane> planeBottom = createClippingPlane(-halfHeight * s, Eigen::Vector3d(sin(params.xbshear())*cos(params.ybshear()),sin(params.ybshear()),-cos(params.xbshear())*cos(params.ybshear())));

        shared_ptr<IfcHalfSpaceSolid> halfSpaceSolid2 ( new IfcHalfSpaceSolid() );
        insertEntity(halfSpaceSolid2);
        halfSpaceSolid2->m_BaseSurface = planeBottom;
        halfSpaceSolid2->m_AgreementFlag = false;

        shared_ptr<IfcBooleanClippingResult> clippingResult2 ( new IfcBooleanClippingResult() );
        insertEntity(clippingResult2);
        clippingResult2->m_Operator = shared_ptr<IfcBooleanOperator >( new IfcBooleanOperator( IfcBooleanOperator::ENUM_DIFFERENCE ) );
        clippingResult2->m_FirstOperand = clippingResult1;
        clippingResult2->m_SecondOperand = halfSpaceSolid2;

        addRepresentationToShape(clippingResult2, shared_ptr<IfcLabel>( new IfcLabel( L"SweptSolid" ) ));
    } else {
        auto sides = RVMMeshHelper2::infoSnoutNumSides(params, m_maxSideSize, m_minSides);
        writeMesh(RVMMeshHelper2::makeSnout(params, sides), matrix);
    }*/
}

void IFCConverter::createCylinder(const std::array<float, 12>& matrix, const Primitives::Cylinder& params) {
    /*if (m_primitives) {
        const auto transform = toEigenTransform(matrix);
        const float s = getScaleFromTransformation(transform);

        shared_ptr<IfcPositiveLengthMeasure> height (new IfcPositiveLengthMeasure() );
        height->m_value = params.height() * s;

        shared_ptr<IfcPositiveLengthMeasure> radius (new IfcPositiveLengthMeasure() );
        radius->m_value = params.radius() * s;

        shared_ptr<IfcCartesianPoint> location( new IfcCartesianPoint() );
        insertEntity(location);
        location->m_Coordinates.push_back( shared_ptr<IfcLengthMeasure>(new IfcLengthMeasure(0.0) ) );
        location->m_Coordinates.push_back( shared_ptr<IfcLengthMeasure>(new IfcLengthMeasure(0.0) ) );

        shared_ptr<IfcAxis2Placement2D> position (new IfcAxis2Placement2D() );
        insertEntity(position);
        position->m_Location = location;

        shared_ptr<IfcCircleProfileDef> profile (new IfcCircleProfileDef() );
        insertEntity(profile);
        profile->m_ProfileType = shared_ptr<IfcProfileTypeEnum>( new IfcProfileTypeEnum( IfcProfileTypeEnum::ENUM_AREA ) );
        profile->m_Position = position;
        profile->m_Radius = radius;

        shared_ptr<IfcDirection> direction (new IfcDirection() );
        insertEntity(direction);
        direction->m_DirectionRatios.push_back(0);
        direction->m_DirectionRatios.push_back(0);
        direction->m_DirectionRatios.push_back(1);

        Eigen::Vector3f offset(0, 0, -params.height() * 0.5f *  s);

        shared_ptr<IfcExtrudedAreaSolid> cylinder (new IfcExtrudedAreaSolid() );
        insertEntity(cylinder);
        cylinder->m_Depth = height;
        cylinder->m_Position = getCoordinateSystem(transform, offset);
        cylinder->m_SweptArea = profile;
        cylinder->m_ExtrudedDirection = direction;

        addRepresentationToShape(cylinder, shared_ptr<IfcLabel>( new IfcLabel( L"SweptSolid" ) ));
    } else {
        auto sides = RVMMeshHelper2::infoCylinderNumSides(params, m_maxSideSize, m_minSides);
        writeMesh(RVMMeshHelper2::makeCylinder(params, sides), matrix);
    }*/
}

void IFCConverter::createSphere(const std::array<float, 12>& matrix, const Primitives::Sphere& params) {
    /*if(m_primitives) {
        Transform3f transform = toEigenTransform(matrix);
        const float s = getScaleFromTransformation(transform);

        shared_ptr<IfcPositiveLengthMeasure> radius (new IfcPositiveLengthMeasure() );
        radius->m_value = params.diameter * 0.5f * s;

        shared_ptr<IfcCartesianPoint> location( new IfcCartesianPoint() );
        insertEntity(location);
        location->m_Coordinates.push_back( shared_ptr<IfcLengthMeasure>(new IfcLengthMeasure(0.0) ) );
        location->m_Coordinates.push_back( shared_ptr<IfcLengthMeasure>(new IfcLengthMeasure(0.0) ) );

        shared_ptr<IfcDirection> direction( new IfcDirection() );
        insertEntity(direction);
        direction->m_DirectionRatios.push_back(0);
        direction->m_DirectionRatios.push_back(1);

        shared_ptr<IfcAxis2Placement2D> position( new IfcAxis2Placement2D() );
        insertEntity(position);
        position->m_Location = location;
        position->m_RefDirection = direction;

        shared_ptr<IfcCircle> circle (new IfcCircle() );
        insertEntity(circle);
        circle->m_Radius = radius;
        circle->m_Position = position;

        shared_ptr<IfcCartesianPoint> p1 (new IfcCartesianPoint() );
        insertEntity(p1);
        p1->m_Coordinates.push_back( shared_ptr<IfcLengthMeasure>(new IfcLengthMeasure( 0.0 ) ) );
        p1->m_Coordinates.push_back( shared_ptr<IfcLengthMeasure>(new IfcLengthMeasure( radius->m_value ) ) );

        shared_ptr<IfcCartesianPoint> p2 (new IfcCartesianPoint() );
        insertEntity(p2);
        p2->m_Coordinates.push_back( shared_ptr<IfcLengthMeasure>(new IfcLengthMeasure( 0.0 ) ) );
        p2->m_Coordinates.push_back( shared_ptr<IfcLengthMeasure>(new IfcLengthMeasure( -radius->m_value ) ) );

        shared_ptr<IfcPolyline> line (new IfcPolyline() );
        insertEntity(line);
        line->m_Points.push_back(p2);
        line->m_Points.push_back(p1);

        shared_ptr<IfcCompositeCurveSegment> segLine (new IfcCompositeCurveSegment() );
        insertEntity(segLine);
        segLine->m_Transition = shared_ptr<IfcTransitionCode>(new IfcTransitionCode( IfcTransitionCode::ENUM_CONTINUOUS ));
        segLine->m_SameSense = true;
        segLine->m_ParentCurve = line;

        //shared_ptr<IfcParameterValue> trim2 (new IfcParameterValue() );
        //trim2->m_value = M_PI;

        shared_ptr<IfcTrimmedCurve> curve (new IfcTrimmedCurve() );
        insertEntity(curve);
        curve->m_BasisCurve = circle;
        curve->m_Trim1.push_back(p1);
        curve->m_Trim2.push_back(p2);
        curve->m_SenseAgreement = false;
        curve->m_MasterRepresentation = shared_ptr<IfcTrimmingPreference>(new IfcTrimmingPreference( IfcTrimmingPreference::ENUM_CARTESIAN ) );

        shared_ptr<IfcCompositeCurveSegment> segCurve (new IfcCompositeCurveSegment() );
        insertEntity(segCurve);
        segCurve->m_Transition = shared_ptr<IfcTransitionCode>(new IfcTransitionCode( IfcTransitionCode::ENUM_CONTINUOUS ));
        segCurve->m_SameSense = true;
        segCurve->m_ParentCurve = curve;

        shared_ptr<IfcCompositeCurve> compositeCurve (new IfcCompositeCurve() );
        insertEntity(compositeCurve);
        compositeCurve->m_Segments.push_back(segLine);
        compositeCurve->m_Segments.push_back(segCurve);
        compositeCurve->m_SelfIntersect = LogicalEnum::LOGICAL_FALSE;


        shared_ptr<IfcArbitraryClosedProfileDef> profile( new IfcArbitraryClosedProfileDef() );
        insertEntity(profile);
        profile->m_ProfileType = shared_ptr<IfcProfileTypeEnum>( new IfcProfileTypeEnum( IfcProfileTypeEnum::ENUM_AREA ) );
        profile->m_OuterCurve = compositeCurve;

        shared_ptr<IfcDirection> axis (new IfcDirection() );
        insertEntity(axis);
        axis->m_DirectionRatios.push_back(0);
        axis->m_DirectionRatios.push_back(1);
        axis->m_DirectionRatios.push_back(0);

        addRevolvedAreaSolidToShape(profile, axis, 2.0 * M_PI, transform);
    } else {
        writeMesh( RVMMeshHelper2::makeSphere(params, m_maxSideSize, m_minSides), matrix);
    }*/
}

void IFCConverter::createLine(const std::array<float, 12>& m, const float& length, const float& thickness) {
    Eigen::Matrix4f matrix = toEigenMatrix(m);
    Eigen::Vector4f origin = matrix.col(3);

    Eigen::Vector4f dir(0.0f, 0.0f, length * thickness * 0.5f, 0.0f);
    dir = matrix * dir;

    Eigen::Vector4f start = origin - dir;
    Eigen::Vector4f end = origin + dir;

    /*shared_ptr<IfcPolyline> line (new IfcPolyline() );
    insertEntity(line);

    shared_ptr<IfcCartesianPoint> startPoint (new IfcCartesianPoint() );
    insertEntity(startPoint);
    startPoint->m_Coordinates.push_back( shared_ptr<IfcLengthMeasure>(new IfcLengthMeasure( start.x() ) ) );
    startPoint->m_Coordinates.push_back( shared_ptr<IfcLengthMeasure>(new IfcLengthMeasure( start.y() ) ) );
    startPoint->m_Coordinates.push_back( shared_ptr<IfcLengthMeasure>(new IfcLengthMeasure( start.z() ) ) );

    line->m_Points.push_back(startPoint);

    shared_ptr<IfcCartesianPoint> endPoint (new IfcCartesianPoint() );
    insertEntity(endPoint);
    endPoint->m_Coordinates.push_back( shared_ptr<IfcLengthMeasure>(new IfcLengthMeasure( end.x() ) ) );
    endPoint->m_Coordinates.push_back( shared_ptr<IfcLengthMeasure>(new IfcLengthMeasure( end.y() ) ) );
    endPoint->m_Coordinates.push_back( shared_ptr<IfcLengthMeasure>(new IfcLengthMeasure( end.z() ) ) );

    line->m_Points.push_back(endPoint);

    addRepresentationToShape(line, shared_ptr<IfcLabel>( new IfcLabel( L"GeometricCurveSet" ) ));*/
}

void IFCConverter::createFacetGroup(const std::array<float, 12>& m, const FGroup& vertices) {
    Eigen::Matrix4f matrix = toEigenMatrix(m);

    /*shared_ptr<IfcConnectedFaceSet> cfs (new IfcConnectedFaceSet() );
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

    addRepresentationToShape(surfaceModel, shared_ptr<IfcLabel>( new IfcLabel( L"SurfaceModel" ) ));
	*/

}

IfcReference IFCConverter::createOwnerHistory(const std::string& user, const std::string& banner, int timeStamp) {
  // The creator/owner of this document
  // https://standards.buildingsmart.org/IFC/RELEASE/IFC2x3/FINAL/HTML/ifcactorresource/lexical/ifcperson.htm
  // -
  IfcEntity person("IFCPERSON");
  person.attributes = {user,
                       user,
                       IFC_STRING_UNSET,
                       IfcStringList(),
                       IfcStringList(),
                       IfcStringList(),
                       IFC_STRING_UNSET,
                       IFC_STRING_UNSET};
  IfcReference personRef = m_writer->addEntity(person);

  IfcEntity organization("IFCORGANIZATION");
  organization.attributes = {IFC_STRING_UNSET, "unknown", IFC_STRING_UNSET, IFC_STRING_UNSET, IFC_STRING_UNSET};
  IfcReference organizationRef = m_writer->addEntity(organization);

  std::string developer = "unknown";
  auto firstSpace = banner.find(" ");
  if (firstSpace != std::string::npos) {
    developer = banner.substr(0, firstSpace);
  }

  // Find a version that starts with Mk and ends at the first string (or at the end of string if no space in string)
  // Version is an empty string if no verison could be found by the rule above
  std::string version = "";
  auto versionStart = banner.find("Mk");
  if (versionStart != std::string::npos) {
    auto versionEnd = banner.find(" ", versionStart);
    if (versionEnd != std::string::npos) {
      // Length of version
      versionEnd -= versionStart;
    }
    version = banner.substr(versionStart, versionEnd);
  }

  // IfcOrganization requires at least a name
  // http://www.buildingsmart-tech.org/ifc/IFC2x3/TC1/html/ifcactorresource/lexical/ifcorganization.htm
  // -
  IfcEntity applicationDeveloper("IFCORGANIZATION");
  ;
  applicationDeveloper.attributes = {IFC_STRING_UNSET, developer, IFC_STRING_UNSET, IFC_STRING_UNSET, IFC_STRING_UNSET};
  IfcReference appOrgRef = m_writer->addEntity(applicationDeveloper);

  // IfcApplication requires ApplicationDeveloper, Version, ApplicationFullName, ApplicationIdentifier
  // https://standards.buildingsmart.org/IFC/RELEASE/IFC2x3/FINAL/HTML/ifcutilityresource/lexical/ifcapplication.htm
  IfcEntity application("IFCAPPLICATION");
  application.attributes = {appOrgRef, version, banner, banner};
  IfcReference owningApplication = m_writer->addEntity(application);

  // IfcPersonAndOrganization requires ThePerson and TheOrganization
  // https://standards.buildingsmart.org/IFC/RELEASE/IFC2x3/FINAL/HTML/ifcactorresource/lexical/ifcpersonandorganization.htm
  // -
  IfcEntity person_org("IFCPERSONANDORGANIZATION");
  person_org.attributes = {personRef, organizationRef, IFC_STRING_UNSET};
  IfcReference owningUser = m_writer->addEntity(person_org);

  // IfcOwnerHistory requires OwningApplication, ChangeAction, and CreationDate
  // https://standards.buildingsmart.org/IFC/RELEASE/IFC2x3/FINAL/HTML/ifcutilityresource/lexical/ifcownerhistory.htm
  // -
  IfcEntity owner_history("IFCOWNERHISTORY");
  owner_history.attributes = {owningUser,       owningApplication, IFC_STRING_UNSET, ".NOCHANGE.",
                              IFC_STRING_UNSET, IFC_STRING_UNSET,  IFC_STRING_UNSET, timeStamp};
  IfcReference ownerRef = m_writer->addEntity(owner_history);

  return ownerRef;
}

void IFCConverter::initModel() {
/*
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
    rel_aggregates_project_site->m_GlobalId = shared_ptr<IfcGloballyUniqueId>( new IfcGloballyUniqueId( CreateCompressedGuidString22() ) );
    rel_aggregates_project_site->m_RelatingObject = m_model->getIfcProject();
    rel_aggregates_project_site->m_RelatedObjects.push_back(site);
    insertEntity(rel_aggregates_project_site);

    // Relation project -> site
    shared_ptr<IfcRelAggregates> rel_aggregates_site_building( new IfcRelAggregates() );
    rel_aggregates_site_building->m_GlobalId = shared_ptr<IfcGloballyUniqueId>( new IfcGloballyUniqueId( CreateCompressedGuidString22() ) );
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
	*/
}

/*void IFCConverter::pushParentRelation(shared_ptr<IfcObjectDefinition> parent) {
    shared_ptr<IfcRelAggregates> child_relations( new IfcRelAggregates() );
    child_relations->m_GlobalId = shared_ptr<IfcGloballyUniqueId>( new IfcGloballyUniqueId( CreateCompressedGuidString22() ) );
    child_relations->m_RelatingObject = parent;
    m_relationStack.push(child_relations);
}*/


void IFCConverter::writeMesh(const Mesh &mesh, const std::array<float, 12>& m) {
   /* shared_ptr<IfcConnectedFaceSet> cfs (new IfcConnectedFaceSet() );
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
    addRepresentationToShape(surfaceModel, shared_ptr<IfcLabel>( new IfcLabel( L"SurfaceModel" ) ));*/
}

/*void IFCConverter::addRepresentationToShape(shared_ptr<IfcRepresentationItem> item, shared_ptr<IfcLabel> type) {
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
            shapeRepresentation->m_RepresentationType = type;

            shape->m_Representations.push_back(shapeRepresentation);
            parentProduct->m_Representation = shape;
        }
        parentProduct->m_Representation->m_Representations[0]->m_Items.push_back(item);

        // Add style to the item
        shared_ptr<IfcPresentationStyleAssignment> presentationStyleAssignment( new IfcPresentationStyleAssignment() );
        insertEntity(presentationStyleAssignment);
        presentationStyleAssignment->m_Styles.push_back(createSurfaceStyle(m_currentMaterial.top()));

        shared_ptr<IfcStyledItem> styledItem( new IfcStyledItem() );
        insertEntity(styledItem);
        styledItem->m_Styles.push_back(presentationStyleAssignment);
        styledItem->m_Item = item;


        shared_ptr<IfcCartesianPoint> location( new IfcCartesianPoint() );
        insertEntity(location);
        location->m_Coordinates.push_back( shared_ptr<IfcLengthMeasure>(new IfcLengthMeasure(0.0) ) );
        location->m_Coordinates.push_back( shared_ptr<IfcLengthMeasure>(new IfcLengthMeasure(0.0) ) );
        location->m_Coordinates.push_back( shared_ptr<IfcLengthMeasure>(new IfcLengthMeasure(0.0) ) );

        shared_ptr<IfcAxis2Placement3D> relative_placement( new IfcAxis2Placement3D() );
        relative_placement->m_Location = location;
        insertEntity(relative_placement);

        // Define a placement based on the relative placement defined above
        // http://www.buildingsmart-tech.org/ifc/IFC2x3/TC1/html/ifcgeometricconstraintresource/lexical/ifclocalplacement.htm
        // -
        shared_ptr<IfcLocalPlacement> placement( new IfcLocalPlacement() );
        insertEntity(placement);
        placement->m_RelativePlacement = relative_placement;

        // IfcProduct.WR1: A product with an representation requires a placement
        parentProduct->m_ObjectPlacement = placement;

    }
}*/

/*void IFCConverter::insertEntity(shared_ptr<IfcPPEntity> e) {
   if(e->m_id < 0) {
        m_currentEntityId++;
        e->m_id = m_currentEntityId;
    }
    m_model->insertEntity(e);
}*/

/*shared_ptr<IfcSurfaceStyle> IFCConverter::createSurfaceStyle(int id) {
    std::map<int, shared_ptr<IfcSurfaceStyle>>::iterator I = m_styles.find(id);
    if(I != m_styles.end()) {
        return (*I).second;
    }

    std::vector<float> colors = RVMColorHelper::color(id);

    shared_ptr<IfcColourRgb> surfaceColor( new IfcColourRgb() );
    insertEntity(surfaceColor);
    surfaceColor->m_Red = getNormalisedRatioMeasure(colors[0]);
    surfaceColor->m_Green = getNormalisedRatioMeasure(colors[1]);
    surfaceColor->m_Blue = getNormalisedRatioMeasure(colors[2]);

    // Define the material proerties for rendering (only the surface color defined through RVM color mappings)
    // http://www.buildingsmart-tech.org/ifc/IFC2x3/TC1/html/ifcpresentationappearanceresource/lexical/ifcsurfacestylerendering.htm
    // -
    shared_ptr<IfcSurfaceStyleRendering> surfaceStyleRendering( new IfcSurfaceStyleRendering() );
    insertEntity(surfaceStyleRendering);
    surfaceStyleRendering->m_SurfaceColour = surfaceColor;
    surfaceStyleRendering->m_Transparency = getNormalisedRatioMeasure(0.0);
    surfaceStyleRendering->m_DiffuseColour = getNormalisedRatioMeasure(1);
    surfaceStyleRendering->m_SpecularColour = getNormalisedRatioMeasure(0.25);
    surfaceStyleRendering->m_ReflectanceMethod = shared_ptr<IfcReflectanceMethodEnum>( new IfcReflectanceMethodEnum( IfcReflectanceMethodEnum::ENUM_BLINN ) );

    shared_ptr<IfcSurfaceStyle> surfaceStyle( new IfcSurfaceStyle() );
    insertEntity(surfaceStyle);
    surfaceStyle->m_Name = shared_ptr<IfcLabel>( new IfcLabel( L"Material" + std::to_wstring(id) + L"Style"));
    surfaceStyle->m_Side = shared_ptr<IfcSurfaceSide>( new IfcSurfaceSide( IfcSurfaceSide::ENUM_BOTH ));
    surfaceStyle->m_Styles.push_back(surfaceStyleRendering);

    return surfaceStyle;
}*/

/*shared_ptr<IfcMaterial> IFCConverter::createMaterial(int id) {
    std::map<int, shared_ptr<IfcMaterial>>::iterator I = m_materials.find(id);
    if(I != m_materials.end()) {
        return (*I).second;
    }
    shared_ptr<IfcMaterial> material( new IfcMaterial() );
    insertEntity(material);
    material->m_Name = shared_ptr<IfcLabel>( new IfcLabel( L"Material" + std::to_wstring(id) ));
    m_materials.insert(std::make_pair(id, material));

    shared_ptr<IfcPresentationStyleAssignment> presentationStyleAssignment( new IfcPresentationStyleAssignment() );
    insertEntity(presentationStyleAssignment);
    presentationStyleAssignment->m_Styles.push_back(createSurfaceStyle(id));

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
}*/

/*void IFCConverter::messageCallBack(void* obj_ptr, shared_ptr<StatusCallback::Message> message) {
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
}*/



/*void IFCConverter::addRevolvedAreaSolidToShape(shared_ptr<IfcProfileDef> profile, shared_ptr<IfcDirection> axis, double angle, const Transform3f& transform) {
            // Now define the axis for revolving
        shared_ptr<IfcCartesianPoint> location( new IfcCartesianPoint() );
        insertEntity(location);
        location->m_Coordinates.push_back( shared_ptr<IfcLengthMeasure>(new IfcLengthMeasure(0.0) ) );
        location->m_Coordinates.push_back( shared_ptr<IfcLengthMeasure>(new IfcLengthMeasure(0.0) ) );
        location->m_Coordinates.push_back( shared_ptr<IfcLengthMeasure>(new IfcLengthMeasure(0.0) ) );

        shared_ptr<IfcAxis1Placement> placement (new IfcAxis1Placement() );
        placement->m_Location = location;
        placement->m_Axis = axis;
        insertEntity(placement);

        shared_ptr<IfcPlaneAngleMeasure> angleP (new IfcPlaneAngleMeasure() );
        angleP->m_value = angle;

        shared_ptr<IfcRevolvedAreaSolid> solid (new IfcRevolvedAreaSolid() );
        insertEntity(solid);
        solid->m_Axis = placement;
        solid->m_Angle = angleP;
        // Rotate the whole geometry (90 degree y-axis) because we defined it with y-up instead of z-up
        solid->m_Position = getCoordinateSystem(transform, Eigen::Vector3f(0,0,0));
        solid->m_SweptArea = profile;

        addRepresentationToShape(solid, shared_ptr<IfcLabel>( new IfcLabel( L"SweptSolid" ) ));
}*/

/*shared_ptr<IfcAxis2Placement3D> IFCConverter::getCoordinateSystem(const Transform3f& matrix, const Eigen::Vector3f& offset) {

    Eigen::Vector3f translation = matrix.translation() + matrix.rotation() * offset;

    shared_ptr<IfcCartesianPoint> location( new IfcCartesianPoint() );
    insertEntity(location);
    location->m_Coordinates.push_back( shared_ptr<IfcLengthMeasure>(new IfcLengthMeasure(translation.x()) ) );
    location->m_Coordinates.push_back( shared_ptr<IfcLengthMeasure>(new IfcLengthMeasure(translation.y()) ) );
    location->m_Coordinates.push_back( shared_ptr<IfcLengthMeasure>(new IfcLengthMeasure(translation.z()) ) );

    Eigen::Matrix3f rotation = matrix.rotation();
    Eigen::Vector3f z_axis = rotation * Eigen::Vector3f(0,0,1);
    Eigen::Vector3f x_axis = rotation * Eigen::Vector3f(1,0,0);

    shared_ptr<IfcDirection> axis (new IfcDirection() );
    insertEntity(axis);
    axis->m_DirectionRatios.push_back(z_axis.x());
    axis->m_DirectionRatios.push_back(z_axis.y());
    axis->m_DirectionRatios.push_back(z_axis.z());

    shared_ptr<IfcDirection> refDirection (new IfcDirection() );
    insertEntity(refDirection);
    refDirection->m_DirectionRatios.push_back(x_axis.x());
    refDirection->m_DirectionRatios.push_back(x_axis.y());
    refDirection->m_DirectionRatios.push_back(x_axis.z());


    shared_ptr<IfcAxis2Placement3D> coordinate_system( new IfcAxis2Placement3D() );
    insertEntity(coordinate_system);
    coordinate_system->m_Location = location;
    coordinate_system->m_Axis = axis;
    coordinate_system->m_RefDirection = refDirection;
    return coordinate_system;
};*/

/*shared_ptr<IfcPlane> IFCConverter::createClippingPlane(double zPos, const Eigen::Vector3d &n) {
    shared_ptr<IfcCartesianPoint> planeLocation( new IfcCartesianPoint() );
    insertEntity(planeLocation);
    planeLocation->m_Coordinates.push_back( shared_ptr<IfcLengthMeasure>(new IfcLengthMeasure(0.0) ) );
    planeLocation->m_Coordinates.push_back( shared_ptr<IfcLengthMeasure>(new IfcLengthMeasure(0.0) ) );
    planeLocation->m_Coordinates.push_back( shared_ptr<IfcLengthMeasure>(new IfcLengthMeasure(zPos) ) );


    shared_ptr<IfcDirection> planeNormal (new IfcDirection() );
    insertEntity(planeNormal);
    planeNormal->m_DirectionRatios.push_back(n.x());
    planeNormal->m_DirectionRatios.push_back(n.y());
    planeNormal->m_DirectionRatios.push_back(n.z());

    // IFCAXIS2PLACEMENT3D
    shared_ptr<IfcAxis2Placement3D> planePosition ( new IfcAxis2Placement3D() );
    insertEntity(planePosition);
    planePosition->m_Location = planeLocation;
    planePosition->m_Axis = planeNormal;

    // IFCPLANE(#44);
    shared_ptr<IfcPlane> plane ( new IfcPlane() );
    insertEntity(plane);
    plane->m_Position = planePosition;

    return plane;
}*/