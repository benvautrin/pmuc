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
#define _USE_MATH_DEFINES  // For PI under VC++
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

#include "../api/rvmcolorhelper.h"
#include "../api/rvmmeshhelper.h"

#include <cassert>
#include <fstream>
#include <string>

using boost::locale::conv::utf_to_utf;

namespace {

Transform3f toEigenTransform(const std::array<float, 12>& matrix) {
  Transform3f result;
  result.setIdentity();
  for (unsigned int i = 0; i < 3; i++) {
    for (unsigned int j = 0; j < 4; j++) {
      result(i, j) = matrix[i + j * 3];
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
  return scale(0, 0);
}
}  // namespace

IFCConverter::IFCConverter(const std::string& filename, const std::string& schema)
    : RVMReader(), m_filename(filename), m_currentEntityId(0) {
  m_writer = new IFCStreamWriter(filename);
  m_writer->startDocument();

  FileDescription desc;
  desc.description.push_back("ViewDefinition [StructuralAnalysisView]");
  desc.description.push_back("PMUC generated IFC file.");

  // 2011-04-21T14:25:12
  std::stringstream wss;
  boost::posix_time::time_facet* facet = new boost::posix_time::time_facet("%Y-%m-%dT%H:%M:%S");
  wss.imbue(std::locale(std::cout.getloc(), facet));
  boost::posix_time::ptime now = boost::posix_time::second_clock::local_time();
  wss << now;
  std::string ts = wss.str();

  FileName name;
  name.name = boost::replace_all_copy(filename, "\\", "\\\\");
  name.time_stamp_text = ts;
  name.preprocessor_version = "IfcPlusPlus";

  m_writer->addHeader(desc, name);
}

IFCConverter::~IFCConverter() {}

void IFCConverter::startDocument() {}

void IFCConverter::endDocument() {
  m_writer->endDocument();
}

void IFCConverter::startHeader(const std::string& banner,
                               const std::string& fileNote,
                               const std::string& dateStr,
                               const std::string& user,
                               const std::string& encoding) {
  createOwnerHistory(user, banner, toTimeStamp(dateStr));

  m_project = new IfcEntity("IFCPROJECT");
  m_project->attributes = {createBase64Uuid<char>(), m_ownerHistory};
}

void IFCConverter::endHeader() {}

void IFCConverter::startModel(const std::string& projectName, const std::string& name) {
  // https://standards.buildingsmart.org/IFC/RELEASE/IFC2x3/FINAL/HTML/ifcgeometryresource/lexical/ifccartesianpoint.htm
  IfcEntity location("IFCCARTESIANPOINT");
  location.attributes = {IfcFloatList{0.0f, 0.0f, 0.0f}};
  IfcReference locationRef = m_writer->addEntity(location);

  IfcEntity world_coordinate_system("IFCAXIS2PLACEMENT3D");
  world_coordinate_system.attributes = {
      locationRef,          // Location
      IFC_REFERENCE_UNSET,  // Axis (Z)
      IFC_REFERENCE_UNSET   // RefDirection (X)
  };
  IfcReference world_coordinate_systemRef = m_writer->addEntity(world_coordinate_system);

  // 3d representation context
  IfcEntity context("IFCGEOMETRICREPRESENTATIONCONTEXT");
  context.attributes = {IFC_STRING_UNSET,  // ContextIdentifier
                        "Model",
                        3,                           // dimension count
                        IFC_STRING_UNSET,            // precision
                        world_coordinate_systemRef,  // WorldCoordinateSystem
                        IFC_REFERENCE_UNSET};
  m_contextRef = m_writer->addEntity(context);

  // IfcUnitAssignment
  // https://standards.buildingsmart.org/IFC/RELEASE/IFC2x3/FINAL/HTML/ifcmeasureresource/lexical/ifcunitassignment.htm

  // length unit [m]
  IfcEntity lenghtUnit("IFCSIUNIT");
  lenghtUnit.attributes = {IFC_REFERENCE_UNSET, IFCUNIT_LENGTHUNIT, IFC_STRING_UNSET, IFCSIUNITNAME_METRE};
  IfcReference lenghtUnitRef = m_writer->addEntity(lenghtUnit);

  // plane unit [rad]
  IfcEntity plane_angle_unit("IFCSIUNIT");
  plane_angle_unit.attributes = {IFC_REFERENCE_UNSET, IFCUNIT_PLANEANGLEUNIT, IFC_STRING_UNSET, IFCSIUNITNAME_RADIAN};
  IfcReference plane_angle_unitRef = m_writer->addEntity(plane_angle_unit);

  // Unit Assignment
  // https://standards.buildingsmart.org/IFC/RELEASE/IFC2x3/FINAL/HTML/ifcmeasureresource/lexical/ifcunitassignment.htm
  IfcEntity unitAssignment("IFCUNITASSIGNMENT");
  unitAssignment.attributes = {IfcReferenceList{lenghtUnitRef, plane_angle_unitRef}};
  IfcReference unitAssignmentRef = m_writer->addEntity(unitAssignment);

  m_project->attributes.push_back(projectName);                     // name
  m_project->attributes.push_back(name);                            // description
  m_project->attributes.push_back(IFC_STRING_UNSET);                // object type
  m_project->attributes.push_back(name);                            // long name
  m_project->attributes.push_back(IFC_STRING_UNSET);                // phase
  m_project->attributes.push_back(IfcReferenceList{m_contextRef});  // RepresentationContexts
  m_project->attributes.push_back(unitAssignmentRef);               // UnitsInContext

  IfcReference projectRef = m_writer->addEntity(*m_project);

  // Initialize model with site, building etc
  initModel(projectRef);
}

void IFCConverter::endModel() {
  assert(m_productStack.size() == 0);
  assert(m_productChildStack.size() == 1);
  createParentChildRelation(m_buildingRef, m_productChildStack.top());
}

void IFCConverter::startGroup(const std::string& name, const Vector3F& translation, const int& materialId) {
  // https://standards.buildingsmart.org/IFC/RELEASE/IFC2x3/FINAL/HTML/ifcproductextension/lexical/ifcbuildingelementproxy.htm

  // TODO: Singleton for placement
  IfcEntity location("IFCCARTESIANPOINT");
  location.attributes = {IfcFloatList{0.0f, 0.0f, 0.0f}};
  auto locationRef = m_writer->addEntity(location);

  IfcEntity relative_placement("IFCAXIS2PLACEMENT3D");
  relative_placement.attributes = {locationRef, IFC_STRING_UNSET, IFC_STRING_UNSET};
  auto relativePlacementRef = m_writer->addEntity(relative_placement);

  // Define a placement based on the relative placement defined above
  // http://www.buildingsmart-tech.org/ifc/IFC2x3/TC1/html/ifcgeometricconstraintresource/lexical/ifclocalplacement.htm
  // -
  IfcEntity placement("IFCLOCALPLACEMENT");
  placement.attributes = {IFC_STRING_UNSET, relativePlacementRef};
  auto placementRef = m_writer->addEntity(placement);

  IfcEntity* buildingElement = new IfcEntity("IFCBUILDINGELEMENTPROXY");
  buildingElement->attributes = {
      createBase64Uuid<char>(),
      m_ownerHistory,    // owner history
      name,              // Name
      IFC_STRING_UNSET,  // Description
      IFC_STRING_UNSET,  // Object Type
      placementRef,      // ObjectPlacement
  };
  m_currentMaterial.push(materialId);
  m_productStack.push(buildingElement);
  m_productChildStack.push(IfcReferenceList{});
  m_productRepresentationStack.push(IfcReferenceList{});
  m_productMetaDataStack.push(IfcReferenceList{});
}

void IFCConverter::createPropertySet(IfcReference relatedObject) {
  assert(!m_productMetaDataStack.empty());

  auto metaData = m_productMetaDataStack.top();

  // Create a property set that holds all properties
  IfcEntity propertySet("IFCPROPERTYSET");
  propertySet.attributes = {createBase64Uuid<char>(),
                            m_ownerHistory,                        // owner history
                            "RVMAttributes",                       // Name
                            "Attributes from RVM Attribute file",  // Description
                            metaData};
  auto propertySetRef = m_writer->addEntity(propertySet);

  // A related object is required (typically a IfcBuildingElementProxy)

  // Now link the created property set with the object
  // https://standards.buildingsmart.org/IFC/RELEASE/IFC2x3/FINAL/HTML/ifckernel/lexical/ifcreldefinesbyproperties.htm
  IfcEntity propertyRelation("IFCRELDEFINESBYPROPERTIES");
  propertyRelation.attributes = {
      createBase64Uuid<char>(),
      m_ownerHistory,                   // owner history
      IFC_STRING_UNSET,                 // Name
      IFC_STRING_UNSET,                 // Description
      IfcReferenceList{relatedObject},  // RelatedObjects
      propertySetRef                    // RelatingPropertyDefinition

  };
  m_writer->addEntity(propertyRelation);
}

void IFCConverter::endGroup() {
  auto buildingElement = m_productStack.top();
  buildingElement->attributes.push_back(createRepresentation());  // Representation
  buildingElement->attributes.push_back(IFC_STRING_UNSET);        // Tag
  buildingElement->attributes.push_back(IFC_REFERENCE_UNSET);     // CompositionType

  IfcReference buildingElementRef = m_writer->addEntity(*buildingElement);
  m_productStack.pop();

  IfcReference material = createMaterial(m_currentMaterial.top());
  m_currentMaterial.pop();

  createPropertySet(buildingElementRef);
  m_productMetaDataStack.pop();

  // https://standards.buildingsmart.org/IFC/RELEASE/IFC2x3/FINAL/HTML/ifcproductextension/lexical/ifcrelassociatesmaterial.htm
  IfcEntity materialAssociates("IFCRELASSOCIATESMATERIAL");
  materialAssociates.attributes = {createBase64Uuid<char>(),
                                   m_ownerHistory,       // owner history
                                   "material_relation",  // Name
                                   IFC_STRING_UNSET,     // Description
                                   IfcReferenceList{buildingElementRef},
                                   material};
  m_writer->addEntity(materialAssociates);

  // Make this group top of the stack
  createParentChildRelation(buildingElementRef, m_productChildStack.top());

  m_productChildStack.pop();
  m_productRepresentationStack.pop();
  m_productChildStack.top().push_back(buildingElementRef);
}

IfcReference IFCConverter::createRepresentation() {
  if (!m_productRepresentationStack.top().size()) {
    return IFC_REFERENCE_UNSET;
  }
  IfcEntity shapeRepresentation("IFCSHAPEREPRESENTATION");
  shapeRepresentation.attributes = {m_contextRef, "Body", "SurfaceModel", m_productRepresentationStack.top()};
  auto shapeRepresentationRef = m_writer->addEntity(shapeRepresentation);

  IfcEntity shape("IFCPRODUCTDEFINITIONSHAPE");
  shape.attributes = {IFC_REFERENCE_UNSET, IFC_REFERENCE_UNSET, IfcReferenceList{shapeRepresentationRef}};
  auto shapeRef = m_writer->addEntity(shape);

  return shapeRef;
}

void IFCConverter::startMetaData() {
}

void IFCConverter::endMetaData() {
}

void IFCConverter::startMetaDataPair(const std::string& name, const std::string& value) {
  assert(m_propertySet);
  if (!m_productMetaDataStack.empty()) {
    // https://standards.buildingsmart.org/IFC/RELEASE/IFC2x3/FINAL/HTML/ifcpropertyresource/lexical/ifcpropertysinglevalue.htm
    IfcEntity prop("IFCPROPERTYSINGLEVALUE");
    prop.attributes = {name, IFC_STRING_UNSET, IfcSimpleValue(value), IFC_STRING_UNSET};
    m_productMetaDataStack.top().push_back(m_writer->addEntity(prop));
  }
}

void IFCConverter::endMetaDataPair() {}

void IFCConverter::createPyramid(const std::array<float, 12>& matrix, const Primitives::Pyramid& params) {
  writeMesh(RVMMeshHelper2::makePyramid(params, m_maxSideSize, m_minSides), matrix);
}

void IFCConverter::createBox(const std::array<float, 12>& matrix, const Primitives::Box& params) {
  if (m_primitives) {
    Transform3f transform = toEigenTransform(matrix);
    float s = getScaleFromTransformation(transform);

    auto locationRef = addCartesianPoint(0, 0);

    IfcEntity position("IFCAXIS2PLACEMENT2D");
    position.attributes = {locationRef, IFC_REFERENCE_UNSET};
    auto positionRef = m_writer->addEntity(position);

    IfcEntity profile("IFCRECTANGLEPROFILEDEF");
    profile.attributes = {IFCPROFILETYPE_AREA, "BOXRECTANGLE", positionRef, params.len[0] * s, params.len[1] * s};
    auto profileRef = m_writer->addEntity(profile);

    auto directionRef = addCartesianPoint(0, 0, 1, "IFCDIRECTION");

    Eigen::Vector3f offset(0, 0, -params.len[2] * 0.5f * s);

    IfcEntity box("IFCEXTRUDEDAREASOLID");
    box.attributes = {profileRef, getCoordinateSystem(toEigenTransform(matrix), offset), directionRef,
                      params.len[2] * s};
    auto boxRef = m_writer->addEntity(box);

    m_productRepresentationStack.top().push_back(boxRef);
    addStyleToItem(boxRef);

    // addRepresentationToShape(box, shared_ptr<IfcLabel>( new IfcLabel( L"SweptSolid" ) ));
  } else {
    writeMesh(RVMMeshHelper2::makeBox(params, m_maxSideSize, m_minSides), matrix);
  }
}

void IFCConverter::createRectangularTorus(const std::array<float, 12>& matrix,
                                          const Primitives::RectangularTorus& params) {
  if (m_primitives) {
    Transform3f transform = toEigenTransform(matrix);
    const float s = getScaleFromTransformation(transform);

    auto yExtend = (params.routside() - params.rinside()) * s;
    auto locationRef = addCartesianPoint(0.0f, params.rinside() * s + 0.5f * yExtend);

    IfcEntity position("IFCAXIS2PLACEMENT2D");
    position.attributes = {locationRef, IFC_REFERENCE_UNSET};
    auto positionRef = m_writer->addEntity(position);

    IfcEntity profile("IFCRECTANGLEPROFILEDEF");
    profile.attributes = {IFCPROFILETYPE_AREA, "BOXRECTANGLE", positionRef, params.height() * s, yExtend};
    auto profileRef = m_writer->addEntity(profile);

    auto directionRef = addCartesianPoint(0, 0, 1, "IFCDIRECTION");
    auto axisRef = addCartesianPoint(1, 0, 0, "IFCDIRECTION");

    addRevolvedAreaSolidToShape(profileRef, axisRef, params.angle(),
                                transform.rotate(Eigen::AngleAxisf(float(0.5 * M_PI), Eigen::Vector3f::UnitY())));
  } else {
    writeMesh(RVMMeshHelper2::makeRectangularTorus(params, m_maxSideSize, m_minSides), matrix);
  }
}

void IFCConverter::createCircularTorus(const std::array<float, 12>& matrix, const Primitives::CircularTorus& params) {
  if (m_primitives) {
    Transform3f transform = toEigenTransform(matrix);
    const float s = getScaleFromTransformation(transform);

    // Define the parametric profile first
    auto radius = params.radius() * s;

    auto locationRef = addCartesianPoint(0.0f, params.offset() * s);

    IfcEntity position("IFCAXIS2PLACEMENT2D");
    position.attributes = {locationRef, IFC_REFERENCE_UNSET};
    auto positionRef = m_writer->addEntity(position);

    IfcEntity profile("IFCCIRCLEPROFILEDEF");
    profile.attributes = {IFCPROFILETYPE_AREA, IFC_STRING_UNSET, positionRef, radius};
    auto profileRef = m_writer->addEntity(profile);

    auto axisRef = addCartesianPoint(0.0f, 0.0f, 1.0f, "IFCDIRECTION");

    addRevolvedAreaSolidToShape(profileRef, axisRef, params.angle(),
                                transform.rotate(Eigen::AngleAxisf(float(0.5 * M_PI), Eigen::Vector3f::UnitY())));
  } else {
    auto sides = RVMMeshHelper2::infoCircularTorusNumSides(params, m_maxSideSize, m_minSides);
    writeMesh(RVMMeshHelper2::makeCircularTorus(params, sides.first, sides.second), matrix);
  }
}

void IFCConverter::createEllipticalDish(const std::array<float, 12>& matrix, const Primitives::EllipticalDish& params) {
  if (m_primitives) {
    Transform3f transform = toEigenTransform(matrix);
    const float s = getScaleFromTransformation(transform);

    float r = params.diameter() * s;
    float r2 = params.radius() * s;

    auto locationRef = addCartesianPoint(0.0f, 0.0);
    auto directionRef = addCartesianPoint(0.0f, 1.0f, "IFCDIRECTION");

    IfcEntity position("IFCAXIS2PLACEMENT2D");
    position.attributes = {locationRef, directionRef};
    auto positionRef = m_writer->addEntity(position);

    IfcEntity ellipse("IFCELLIPSE");
    ellipse.attributes = {positionRef, r2, r};
    auto ellipseRef = m_writer->addEntity(ellipse);

    auto p1 = addCartesianPoint(r, 0.0);
    auto p2 = addCartesianPoint(0.0, 0.0);
    auto p3 = addCartesianPoint(0.0, r2);

    IfcEntity line("IFCPOLYLINE");
    line.attributes = {IfcReferenceList{p1, p2, p3}};
    auto lineRef = m_writer->addEntity(line);

    IfcEntity segLine("IFCCOMPOSITECURVESEGMENT");
    segLine.attributes = {IFCTRANSITIONCODE_CONTINUOUS, IFC_TRUE, lineRef};
    auto segLineRef = m_writer->addEntity(segLine);

    IfcEntity curve("IFCTRIMMEDCURVE");
    curve.attributes = {ellipseRef, IfcReferenceList{p3}, IfcReferenceList{p1}, IFC_FALSE,
                        IFCTRIMMINGPREFERENCE_CARTESIAN};
    auto curveRef = m_writer->addEntity(curve);

    IfcEntity segCurve("IFCCOMPOSITECURVESEGMENT");
    segCurve.attributes = {IFCTRANSITIONCODE_CONTINUOUS, IFC_TRUE, curveRef};
    auto segCurveRef = m_writer->addEntity(segCurve);

    IfcEntity compositeCurve("IFCCOMPOSITECURVE");
    compositeCurve.attributes = {IfcReferenceList{segLineRef, segCurveRef}, IFC_FALSE};
    auto compositeCurveRef = m_writer->addEntity(compositeCurve);

    IfcEntity profile("IFCARBITRARYCLOSEDPROFILEDEF");
    profile.attributes = {IFCPROFILETYPE_AREA, IFC_STRING_UNSET, compositeCurveRef};
    auto profileRef = m_writer->addEntity(profile);

    auto axis = addCartesianPoint(0.0f, 1.0f, 0.0f, "IFCDIRECTION");

    addRevolvedAreaSolidToShape(profileRef, axis, float(2.0 * M_PI),
                                transform.rotate(Eigen::AngleAxisf(float(0.5 * M_PI), Eigen::Vector3f::UnitX())));
  } else {
    auto sides = RVMMeshHelper2::infoEllipticalDishNumSides(params, m_maxSideSize, m_minSides);
    writeMesh(RVMMeshHelper2::makeEllipticalDish(params, sides.first, sides.second), matrix);
  }
}

void IFCConverter::createSphericalDish(const std::array<float, 12>& matrix, const Primitives::SphericalDish& params) {
  if (m_primitives) {
    Transform3f transform = toEigenTransform(matrix);
    const float s = getScaleFromTransformation(transform);

    float radius = params.diameter() * 0.5f * s;
    float h = params.height() * s;
    float offset = radius - h;
    float angle = asin(1.0f - h / radius);

    auto locationRef = addCartesianPoint(0.0f, -offset);
    auto directionRef = addCartesianPoint(0.0f, 1.0f, "IFCDIRECTION");

    IfcEntity position("IFCAXIS2PLACEMENT2D");
    position.attributes = {locationRef, directionRef};
    auto positionRef = m_writer->addEntity(position);

    IfcEntity circle("IFCCIRCLE");
    circle.attributes = {positionRef, radius};
    auto circleRef = m_writer->addEntity(circle);

    auto p1 = addCartesianPoint(radius * cos(angle), radius * sin(angle) - offset);
    auto p2 = addCartesianPoint(0.0, 0.0);
    auto p3 = addCartesianPoint(0.0, h);

    IfcEntity line("IFCPOLYLINE");
    line.attributes = {IfcReferenceList{p1, p2, p3}};
    auto lineRef = m_writer->addEntity(line);

    IfcEntity segLine("IFCCOMPOSITECURVESEGMENT");
    segLine.attributes = {IFCTRANSITIONCODE_CONTINUOUS, IFC_TRUE, lineRef};
    auto segLineRef = m_writer->addEntity(segLine);

    IfcEntity curve("IFCTRIMMEDCURVE");
    curve.attributes = {circleRef, IfcReferenceList{p3}, IfcReferenceList{p1}, IFC_FALSE,
                        IFCTRIMMINGPREFERENCE_CARTESIAN};
    auto curveRef = m_writer->addEntity(curve);

    IfcEntity segCurve("IFCCOMPOSITECURVESEGMENT");
    segCurve.attributes = {IFCTRANSITIONCODE_CONTINUOUS, IFC_TRUE, curveRef};
    auto segCurveRef = m_writer->addEntity(segCurve);

    IfcEntity compositeCurve("IFCCOMPOSITECURVE");
    compositeCurve.attributes = {IfcReferenceList{segLineRef, segCurveRef}, IFC_FALSE};
    auto compositeCurveRef = m_writer->addEntity(compositeCurve);

    IfcEntity profile("IFCARBITRARYCLOSEDPROFILEDEF");
    profile.attributes = {IFCPROFILETYPE_AREA, IFC_STRING_UNSET, compositeCurveRef};
    auto profileRef = m_writer->addEntity(profile);

    auto axis = addCartesianPoint(0.0f, 1.0f, 0.0f, "IFCDIRECTION");

    addRevolvedAreaSolidToShape(profileRef, axis, float(2.0 * M_PI),
                                transform.rotate(Eigen::AngleAxisf(float(0.5 * M_PI), Eigen::Vector3f::UnitX())));
  } else {
    writeMesh(RVMMeshHelper2::makeSphericalDish(params, m_maxSideSize, m_minSides), matrix);
  }
}

void IFCConverter::createSnout(const std::array<float, 12>& matrix, const Primitives::Snout& params) {
  if (params.xtshear() > FLT_EPSILON || params.ytshear() > FLT_EPSILON || params.xbshear() > FLT_EPSILON ||
      params.ybshear() > FLT_EPSILON) {
    createSlopedCylinder(matrix, params);
  } else {
    auto sides = RVMMeshHelper2::infoSnoutNumSides(params, m_maxSideSize, m_minSides);
    writeMesh(RVMMeshHelper2::makeSnout(params, sides), matrix);
  }
}

void IFCConverter::createSlopedCylinder(const std::array<float, 12>& matrix, const Primitives::Snout& params) {
  if (m_primitives && std::abs(params.dtop() - params.dbottom()) < std::numeric_limits<float>::epsilon()) {
    const auto transform = toEigenTransform(matrix);
    const float s = getScaleFromTransformation(transform);

    const float r = params.dtop();
    const float halfHeight = params.height() * 0.5f;
    const float topOffset = r * std::max(std::abs(tan(params.xtshear())), std::abs(tan(params.ytshear())));
    const float bottomOffset = r * std::max(std::abs(tan(params.xtshear())), std::abs(tan(params.ytshear())));

    auto height = (params.height() + topOffset + bottomOffset) * s;
    auto radius = r * s;

    auto locationRef = addCartesianPoint(0.0, 0.0);

    IfcEntity position("IFCAXIS2PLACEMENT2D");
    position.attributes = {locationRef, IFC_REFERENCE_UNSET};
    auto positionRef = m_writer->addEntity(position);

    IfcEntity profile("IFCCIRCLEPROFILEDEF");
    profile.attributes = {IFCPROFILETYPE_AREA, IFC_STRING_UNSET, positionRef, radius};
    auto profileRef = m_writer->addEntity(profile);

    auto directionRef = addCartesianPoint(0, 0, 1, "IFCDIRECTION");

    Eigen::Vector3f offset(0, 0, -(halfHeight + bottomOffset) * s);

    IfcEntity cylinder("IFCEXTRUDEDAREASOLID");
    cylinder.attributes = {profileRef, getCoordinateSystem(transform, offset), directionRef, height};
    auto cylinderRef = m_writer->addEntity(cylinder);

    IfcReference planeTopRef = createClippingPlane(
        halfHeight * s, Eigen::Vector3f(-sin(params.xtshear()) * cos(params.ytshear()), -sin(params.ytshear()),
                                        cos(params.xtshear()) * cos(params.ytshear())));

    IfcEntity halfSpaceSolid("IFCHALFSPACESOLID");
    halfSpaceSolid.attributes = {planeTopRef, IFC_FALSE};
    auto halfSpaceSolidRef = m_writer->addEntity(halfSpaceSolid);

    IfcEntity clippingResult1("IFCBOOLEANCLIPPINGRESULT");
    clippingResult1.attributes = {IFCBOOLEANOPERATOR_DIFFERENCE, cylinderRef, halfSpaceSolidRef};
    auto clippingResult1Ref = m_writer->addEntity(clippingResult1);

    IfcReference planeBottomRef = createClippingPlane(
        -halfHeight * s, Eigen::Vector3f(sin(params.xbshear()) * cos(params.ybshear()), sin(params.ybshear()),
                                         -cos(params.xbshear()) * cos(params.ybshear())));

    IfcEntity halfSpaceSolid2("IFCHALFSPACESOLID");
    halfSpaceSolid2.attributes = {planeBottomRef, IFC_FALSE};
    auto halfSpaceSolid2Ref = m_writer->addEntity(halfSpaceSolid2);

    IfcEntity clippingResult2("IFCBOOLEANCLIPPINGRESULT");
    clippingResult2.attributes = {IFCBOOLEANOPERATOR_DIFFERENCE, clippingResult1Ref, halfSpaceSolid2Ref};
    auto clippingResult2Ref = m_writer->addEntity(clippingResult2);

    m_productRepresentationStack.top().push_back(clippingResult2Ref);
    addStyleToItem(clippingResult2Ref);

    // addRepresentationToShape(clippingResult2, shared_ptr<IfcLabel>(new IfcLabel(L"SweptSolid")));
  } else {
    auto sides = RVMMeshHelper2::infoSnoutNumSides(params, m_maxSideSize, m_minSides);
    writeMesh(RVMMeshHelper2::makeSnout(params, sides), matrix);
  }
}

void IFCConverter::createCylinder(const std::array<float, 12>& matrix, const Primitives::Cylinder& params) {
  if (m_primitives) {
    const auto transform = toEigenTransform(matrix);
    const float s = getScaleFromTransformation(transform);

    auto height = params.height() * s;
    auto radius = params.radius() * s;

    auto locationRef = addCartesianPoint(0.0, 0.0);

    IfcEntity position("IFCAXIS2PLACEMENT2D");
    position.attributes = {locationRef, IFC_REFERENCE_UNSET};
    auto positionRef = m_writer->addEntity(position);

    IfcEntity profile("IFCCIRCLEPROFILEDEF");
    profile.attributes = {IFCPROFILETYPE_AREA, IFC_STRING_UNSET, positionRef, radius};
    auto profileRef = m_writer->addEntity(profile);

    auto directionRef = addCartesianPoint(0, 0, 1, "IFCDIRECTION");

    Eigen::Vector3f offset(0, 0, -params.height() * 0.5f * s);

    IfcEntity cylinder("IFCEXTRUDEDAREASOLID");
    cylinder.attributes = {profileRef, getCoordinateSystem(transform, offset), directionRef, height};
    auto cylinderRef = m_writer->addEntity(cylinder);

    m_productRepresentationStack.top().push_back(cylinderRef);
    addStyleToItem(cylinderRef);

    // addRepresentationToShape(cylinder, shared_ptr<IfcLabel>(new IfcLabel(L"SweptSolid")));
  } else {
    auto sides = RVMMeshHelper2::infoCylinderNumSides(params, m_maxSideSize, m_minSides);
    writeMesh(RVMMeshHelper2::makeCylinder(params, sides), matrix);
  }
}

void IFCConverter::createSphere(const std::array<float, 12>& matrix, const Primitives::Sphere& params) {
  if (m_primitives) {
    Transform3f transform = toEigenTransform(matrix);
    const float s = getScaleFromTransformation(transform);
    auto radius = params.diameter * 0.5f * s;

    auto locationRef = addCartesianPoint(0, 0);
    auto directionRef = addCartesianPoint(0, 1, "IFCDIRECTION");

    IfcEntity position("IFCAXIS2PLACEMENT2D");
    position.attributes = {locationRef, directionRef};
    auto positionRef = m_writer->addEntity(position);

    IfcEntity circle("IFCCIRCLE");
    circle.attributes = {positionRef, radius};
    auto circleRef = m_writer->addEntity(circle);

    auto p1 = addCartesianPoint(0.0, radius);
    auto p2 = addCartesianPoint(0.0, -radius);

    IfcEntity line("IFCPOLYLINE");
    line.attributes = {IfcReferenceList{p1, p2}};
    auto lineRef = m_writer->addEntity(line);

    IfcEntity segLine("IFCCOMPOSITECURVESEGMENT");
    segLine.attributes = {IFCTRANSITIONCODE_CONTINUOUS, IFC_TRUE, lineRef};
    auto segLineRef = m_writer->addEntity(segLine);

    // shared_ptr<IfcParameterValue> trim2 (new IfcParameterValue() );
    // trim2->m_value = M_PI;

    IfcEntity curve("IFCTRIMMEDCURVE");
    curve.attributes = {circleRef, IfcReferenceList{p1}, IfcReferenceList{p2}, IFC_FALSE,
                        IFCTRIMMINGPREFERENCE_CARTESIAN};
    auto curveRef = m_writer->addEntity(curve);

    IfcEntity segCurve("IFCCOMPOSITECURVESEGMENT");
    segCurve.attributes = {IFCTRANSITIONCODE_CONTINUOUS, IFC_TRUE, curveRef};
    auto segCurveRef = m_writer->addEntity(segCurve);

    IfcEntity compositeCurve("IFCCOMPOSITECURVE");
    compositeCurve.attributes = {IfcReferenceList{segLineRef, segCurveRef}, IFC_FALSE};
    auto compositeCurveRef = m_writer->addEntity(compositeCurve);

    IfcEntity profile("IFCARBITRARYCLOSEDPROFILEDEF");
    profile.attributes = {IFCPROFILETYPE_AREA, IFC_STRING_UNSET, compositeCurveRef};
    auto profileRef = m_writer->addEntity(profile);

    auto axisRef = addCartesianPoint(0, 1, 0, "IFCDIRECTION");

    addRevolvedAreaSolidToShape(profileRef, axisRef, 2.0 * (float)M_PI, transform);
  } else {
    writeMesh(RVMMeshHelper2::makeSphere(params, m_maxSideSize, m_minSides), matrix);
  }
}

void IFCConverter::createLine(const std::array<float, 12>& m, const float& length, const float& thickness) {
  Eigen::Matrix4f matrix = toEigenMatrix(m);
  Eigen::Vector4f origin = matrix.col(3);

  Eigen::Vector4f dir(0.0f, 0.0f, length * thickness * 0.5f, 0.0f);
  dir = matrix * dir;

  Eigen::Vector4f start = origin - dir;
  Eigen::Vector4f end = origin + dir;

  IfcEntity startPoint("IFCCARTESIANPOINT");
  startPoint.attributes = {IfcFloatList{start.x(), start.y(), start.z()}};
  auto startRef = m_writer->addEntity(startPoint);

  IfcEntity endPoint("IFCCARTESIANPOINT");
  endPoint.attributes = {IfcFloatList{end.x(), end.y(), end.z()}};
  auto endRef = m_writer->addEntity(endPoint);

  IfcEntity line("IFCPOLYLINE");
  line.attributes = {IfcReferenceList{startRef, endRef}};
  auto lineRef = m_writer->addEntity(line);

  m_productRepresentationStack.top().push_back(lineRef);
  addStyleToItem(lineRef);
}

void IFCConverter::createFacetGroup(const std::array<float, 12>& m, const FGroup& vertices) {
  Eigen::Matrix4f matrix = toEigenMatrix(m);

  IfcReferenceList faceSet;
  for (unsigned int i = 0; i < vertices.size(); i++) {
    IfcReferenceList boundList;
    for (unsigned int j = 0; j < vertices[i].size(); j++) {
      IfcReferenceList vertexList;
      for (unsigned int k = 0; k < vertices[i][j].size(); k++) {
        // Transform vertex
        Vector3F v = vertices[i][j].at(k).first;
        Eigen::Vector4f vertex(v.x(), v.y(), v.z(), 1.0f);
        vertex = matrix * vertex;

        auto pointRef = addCartesianPoint(vertex.x(), vertex.y(), vertex.z());
        vertexList.push_back(pointRef);
      }
      // Add face to faceset
      IfcEntity polygon("IFCPOLYLOOP");
      polygon.attributes = {vertexList};
      auto polygonRef = m_writer->addEntity(polygon);

      IfcEntity bound("IFCFACEBOUND");
      bound.attributes = {polygonRef, IFC_FALSE};
      auto boundRef = m_writer->addEntity(bound);

      boundList.push_back(boundRef);
    }

    IfcEntity face("IFCFACE");
    face.attributes = {boundList};
    auto faceRef = m_writer->addEntity(face);

    // Add face to faceset
    faceSet.push_back(faceRef);
  }

  IfcEntity cfs("IFCCONNECTEDFACESET");
  cfs.attributes = {faceSet};
  auto csfRef = m_writer->addEntity(cfs);

  IfcEntity surfaceModel("IFCFACEBASEDSURFACEMODEL");
  surfaceModel.attributes = {IfcReferenceList{csfRef}};
  auto surfaceModelRef = m_writer->addEntity(surfaceModel);

  m_productRepresentationStack.top().push_back(surfaceModelRef);
  addStyleToItem(surfaceModelRef);
  // addRepresentationToShape(surfaceModel, shared_ptr<IfcLabel>( new IfcLabel( L"SurfaceModel" ) ));
}

void IFCConverter::createOwnerHistory(const std::string& user, const std::string& banner, int timeStamp) {
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
  owner_history.attributes = {owningUser,       owningApplication, IFC_STRING_UNSET, IFCCHANGEACTIONENUM_NOCHANGE,
                              IFC_STRING_UNSET, IFC_STRING_UNSET,  IFC_STRING_UNSET, timeStamp};
  m_ownerHistory = m_writer->addEntity(owner_history);
}

void IFCConverter::initModel(const IfcReference projectRef) {
  // create Site
  // https://standards.buildingsmart.org/IFC/RELEASE/IFC2x3/FINAL/HTML/ifcproductextension/lexical/ifcsite.htm
  IfcEntity site("IFCSITE");
  site.attributes = {
      createBase64Uuid<char>(),
      m_ownerHistory,                 // owner history
      "Site",                         // Name
      IFC_STRING_UNSET,               // Description
      IFC_STRING_UNSET,               // Object Type
      IFC_REFERENCE_UNSET,            // ObjectPlacement
      IFC_REFERENCE_UNSET,            // Representation
      IFC_STRING_UNSET,               // LongName
      IFCELEMENTCOMPOSITION_ELEMENT,  // CompositionType
      IFC_STRING_UNSET,               // RefLatitude
      IFC_STRING_UNSET,               // RefLongitude
      IFC_STRING_UNSET,               // RefElevation
      IFC_STRING_UNSET,               // LandTitleNumber
      IFC_STRING_UNSET,               // SiteAddress
  };
  IfcReference siteRef = m_writer->addEntity(site);

  // create Building
  // https://standards.buildingsmart.org/IFC/RELEASE/IFC2x3/FINAL/HTML/ifcproductextension/lexical/ifcbuilding.htm
  IfcEntity building("IFCBUILDING");
  building.attributes = {
      createBase64Uuid<char>(),
      m_ownerHistory,                 // owner history
      "Building",                     // Name
      IFC_STRING_UNSET,               // Description
      IFC_STRING_UNSET,               // Object Type
      IFC_REFERENCE_UNSET,            // ObjectPlacement
      IFC_REFERENCE_UNSET,            // Representation
      IFC_STRING_UNSET,               // LongName
      IFCELEMENTCOMPOSITION_ELEMENT,  // CompositionType
      IFC_STRING_UNSET,               // ElevationOfRefHeight
      IFC_STRING_UNSET,               // ElevationOfTerrain
      IFC_STRING_UNSET,               // BuildingAddress
  };
  m_buildingRef = m_writer->addEntity(building);
  m_productChildStack.push(IfcReferenceList{});

  // Relation project -> site
  // https://standards.buildingsmart.org/IFC/RELEASE/IFC2x3/FINAL/HTML/ifckernel/lexical/ifcrelaggregates.htm
  IfcEntity rel_aggregates_project_site("IFCRELAGGREGATES");
  rel_aggregates_project_site.attributes = {
      createBase64Uuid<char>(),
      m_ownerHistory,            // owner history
      IFC_STRING_UNSET,          // Name
      IFC_STRING_UNSET,          // Description
      projectRef,                // RelatingObject
      IfcReferenceList{siteRef}  // RelatedObjects
  };
  m_writer->addEntity(rel_aggregates_project_site);

  // Relation site -> building
  // https://standards.buildingsmart.org/IFC/RELEASE/IFC2x3/FINAL/HTML/ifckernel/lexical/ifcrelaggregates.htm
  IfcEntity rel_aggregates_site_building("IFCRELAGGREGATES");
  rel_aggregates_site_building.attributes = {
      createBase64Uuid<char>(),
      m_ownerHistory,                  // owner history
      IFC_STRING_UNSET,                // Name
      IFC_STRING_UNSET,                // Description
      siteRef,                         // RelatingObject
      IfcReferenceList{m_buildingRef}  // RelatedObjects
  };
  m_writer->addEntity(rel_aggregates_site_building);
}

void IFCConverter::createParentChildRelation(const IfcReference parent, const IfcReferenceList& children) {
  if (children.empty()) {
    return;
  }
  IfcEntity child_relations = IfcEntity("IFCRELAGGREGATES");
  child_relations.attributes = {
      createBase64Uuid<char>(),
      m_ownerHistory,    // owner history
      IFC_STRING_UNSET,  // Name
      IFC_STRING_UNSET,  // Description
      parent,            // RelatingObject
      children           // RelatedObjects
  };
  m_writer->addEntity(child_relations);
}

void IFCConverter::writeMesh(const Mesh& mesh, const std::array<float, 12>& m) {
  Eigen::Matrix4f matrix = toEigenMatrix(m);

  IfcReferenceList faceSet;
  // For each triangle
  for (unsigned int i = 0; i < mesh.positionIndex.size() / 3; i++) {
    IfcReferenceList vertexList;
    for (unsigned int j = 0; j < 3; j++) {
      unsigned long idx = mesh.positionIndex.at(i * 3 + j);
      // Transform vertex
      Vector3F v = mesh.positions.at(idx);
      Eigen::Vector4f vertex(v.x(), v.y(), v.z(), 1.0f);
      vertex = matrix * vertex;

      vertexList.push_back(addCartesianPoint(vertex.x(), vertex.y(), vertex.z()));
    }
    IfcEntity polygon("IFCPOLYLOOP");
    polygon.attributes = {vertexList};
    auto polygonRef = m_writer->addEntity(polygon);

    IfcEntity bound("IFCFACEBOUND");
    bound.attributes = {polygonRef, IFC_FALSE};
    auto boundRef = m_writer->addEntity(bound);

    IfcEntity face("IFCFACE");
    face.attributes = {IfcReferenceList{boundRef}};
    auto faceRef = m_writer->addEntity(face);

    // Add face to faceset
    faceSet.push_back(faceRef);
  }

  IfcEntity cfs("IFCCONNECTEDFACESET");
  cfs.attributes = {faceSet};
  auto csfRef = m_writer->addEntity(cfs);

  IfcEntity surfaceModel("IFCFACEBASEDSURFACEMODEL");
  surfaceModel.attributes = {IfcReferenceList{csfRef}};
  auto surfaceModelRef = m_writer->addEntity(surfaceModel);

  m_productRepresentationStack.top().push_back(surfaceModelRef);
  addStyleToItem(surfaceModelRef);
}

void IFCConverter::addStyleToItem(IfcReference item) {
  // Add style to the item
  IfcEntity presentationStyleAssignment("IFCPRESENTATIONSTYLEASSIGNMENT");
  presentationStyleAssignment.attributes = {IfcReferenceList{createSurfaceStyle(m_currentMaterial.top())}};
  auto presentationStyleAssignmentRef = m_writer->addEntity(presentationStyleAssignment);

  IfcEntity styledItem("IFCSTYLEDITEM");
  styledItem.attributes = {item, IfcReferenceList{presentationStyleAssignmentRef}, IFC_STRING_UNSET};
  m_writer->addEntity(styledItem);
}

/*void IFCConverter::insertEntity(shared_ptr<IfcPPEntity> e) {
   if(e->m_id < 0) {
        m_currentEntityId++;
        e->m_id = m_currentEntityId;
    }
    m_model->insertEntity(e);
}*/

IfcReference IFCConverter::createSurfaceStyle(int id) {
  auto I = m_styles.find(id);
  if (I != m_styles.end()) {
    return (*I).second;
  }

  std::vector<float> colors = RVMColorHelper::color(id);

  // https://standards.buildingsmart.org/IFC/RELEASE/IFC2x3/FINAL/HTML/ifcpresentationresource/lexical/ifccolourrgb.htm
  IfcEntity surfaceColor("IFCCOLOURRGB");
  surfaceColor.attributes = {std::to_string(id), colors[0], colors[1], colors[2]};
  auto surfaceColorRef = m_writer->addEntity(surfaceColor);

  // Define the material proerties for rendering (only the surface color defined through RVM color mappings)
  // https://standards.buildingsmart.org/IFC/RELEASE/IFC2x3/FINAL/HTML/ifcpresentationappearanceresource/lexical/ifcsurfacestylerendering.htm
  // -
  IfcEntity surfaceStyleRendering("IFCSURFACESTYLERENDERING");
  surfaceStyleRendering.attributes = {
      surfaceColorRef,
      0.0f,                                                 // Transparency
      IfcSimpleValue("1.0", "IFCNORMALISEDRATIOMEASURE"),   // Diffuse Color (multiplicator)
      IFC_STRING_UNSET,                                     // TransmissionColour
      IFC_STRING_UNSET,                                     // DiffuseTransmissionColour
      IFC_STRING_UNSET,                                     // ReflectionColour
      IfcSimpleValue("0.25", "IFCNORMALISEDRATIOMEASURE"),  // Diffuse Color (multiplicator)
      IFC_STRING_UNSET,                                     // SpecuarHighlight
      IFCREFLECTANCEMETHOD_BLINN};
  auto surfaceStyleRenderingRef = m_writer->addEntity(surfaceStyleRendering);

  IfcEntity surfaceStyle("IFCSURFACESTYLE");
  surfaceStyle.attributes = {"Material" + std::to_string(id) + "Style", IFCSURFACESIDE_BOTH,
                             IfcReferenceList{surfaceStyleRenderingRef}};
  auto surfaceStyleRef = m_writer->addEntity(surfaceStyle);

  return surfaceStyleRef;
}

IfcReference IFCConverter::createMaterial(int id) {
  auto I = m_materials.find(id);
  if (I != m_materials.end()) {
    return (*I).second;
  }

  // https://standards.buildingsmart.org/IFC/RELEASE/IFC2x3/FINAL/HTML/ifcmaterialresource/lexical/ifcmaterial.htm
  IfcEntity material("IFCMATERIAL");
  material.attributes = {"Material" + std::to_string(id), IFC_STRING_UNSET, IFC_STRING_UNSET};
  auto materialRef = m_writer->addEntity(material);
  m_materials.insert(std::make_pair(id, materialRef));

  // https://standards.buildingsmart.org/IFC/RELEASE/IFC2x3/FINAL/HTML/ifcpresentationappearanceresource/lexical/ifcpresentationstyleassignment.htm
  IfcEntity presentationStyleAssignment("IFCPRESENTATIONSTYLEASSIGNMENT");
  presentationStyleAssignment.attributes = {IfcReferenceList{createSurfaceStyle(id)}};
  auto presentationStyleAssignmentRef = m_writer->addEntity(presentationStyleAssignment);

  // https://standards.buildingsmart.org/IFC/RELEASE/IFC2x3/FINAL/HTML/ifcpresentationappearanceresource/lexical/ifcstyleditem.htm
  IfcEntity styledItem("IFCSTYLEDITEM");
  styledItem.attributes = {IFC_REFERENCE_UNSET, IfcReferenceList{presentationStyleAssignmentRef}, IFC_STRING_UNSET};
  auto styledItemRef = m_writer->addEntity(styledItem);

  // https://standards.buildingsmart.org/IFC/RELEASE/IFC2x3/FINAL/HTML/ifcrepresentationresource/lexical/ifcstyledrepresentation.htm
  IfcEntity styledRepresentation("IFCSTYLEDREPRESENTATION");
  styledRepresentation.attributes = {m_contextRef,
                                     IFC_STRING_UNSET,  // RepresentationMap
                                     IFC_STRING_UNSET,  // LayerAssignments
                                     IfcReferenceList{styledItemRef}};
  auto styledRepresentationRef = m_writer->addEntity(styledRepresentation);

  // https://standards.buildingsmart.org/IFC/RELEASE/IFC2x3/FINAL/HTML/ifcrepresentationresource/lexical/ifcmaterialdefinitionrepresentation.htm
  IfcEntity materialDefinition("IFCMATERIALDEFINITIONREPRESENTATION");
  materialDefinition.attributes = {IFC_STRING_UNSET,                           // Name
                                   IFC_STRING_UNSET,                           // Description
                                   IfcReferenceList{styledRepresentationRef},  // Representations
                                   materialRef};
  m_writer->addEntity(materialDefinition);

  return materialRef;
}

void IFCConverter::addRevolvedAreaSolidToShape(IfcReference profileRef,
                                               IfcReference axixRef,
                                               float angle,
                                               const Transform3f& transform) {
  // Now define the axis for revolving
  auto locationRef = addCartesianPoint(0, 0, 0);

  IfcEntity placement("IFCAXIS1PLACEMENT");
  placement.attributes = {locationRef, axixRef};
  auto placementRef = m_writer->addEntity(placement);

  // Rotate the whole geometry (90 degree y-axis) because we defined it with y-up instead of z-up
  IfcEntity solid("IFCREVOLVEDAREASOLID");
  solid.attributes = {profileRef, getCoordinateSystem(transform, Eigen::Vector3f(0, 0, 0)), placementRef, angle};
  auto solidRef = m_writer->addEntity(solid);

  m_productRepresentationStack.top().push_back(solidRef);
  addStyleToItem(solidRef);
  // addRepresentationToShape(solid, shared_ptr<IfcLabel>( new IfcLabel( L"SweptSolid" ) ));
}

IfcReference IFCConverter::addCartesianPoint(float x, float y, float z, std::string entity) {
  IfcEntity point(entity);
  point.attributes = {IfcFloatList{x, y, z}};
  return m_writer->addEntity(point);
}

IfcReference IFCConverter::addCartesianPoint(float x, float y, std::string entity) {
  IfcEntity point(entity);
  point.attributes = {IfcFloatList{x, y}};
  return m_writer->addEntity(point);
}

IfcReference IFCConverter::getCoordinateSystem(const Transform3f& matrix, const Eigen::Vector3f& offset) {
  Eigen::Vector3f translation = matrix.translation() + matrix.rotation() * offset;

  IfcReference locationRef = addCartesianPoint(translation.x(), translation.y(), translation.z());

  Eigen::Matrix3f rotation = matrix.rotation();
  Eigen::Vector3f z_axis = rotation * Eigen::Vector3f(0, 0, 1);
  Eigen::Vector3f x_axis = rotation * Eigen::Vector3f(1, 0, 0);

  IfcReference directionRef = addCartesianPoint(z_axis.x(), z_axis.y(), z_axis.z(), "IFCDIRECTION");
  IfcReference refDirectionRef = addCartesianPoint(x_axis.x(), x_axis.y(), x_axis.z(), "IFCDIRECTION");

  IfcEntity coordinate_system("IFCAXIS2PLACEMENT3D");
  coordinate_system.attributes = {locationRef, directionRef, refDirectionRef};
  return m_writer->addEntity(coordinate_system);
};

IfcReference IFCConverter::createClippingPlane(float zPos, const Eigen::Vector3f& n) {
  auto planeLocationRef = addCartesianPoint(0.0f, 0.0f, zPos);
  auto planeNormalRef = addCartesianPoint(n.x(), n.y(), n.z(), "IFCDIRECTION");

  // IFCAXIS2PLACEMENT3D
  IfcEntity planePosition("IFCAXIS2PLACEMENT3D");
  planePosition.attributes = {planeLocationRef, planeNormalRef, IFC_REFERENCE_UNSET};
  auto planePositionRef = m_writer->addEntity(planePosition);

  // IFCPLANE(#44);
  IfcEntity plane("IFCPLANE");
  plane.attributes = {planePositionRef};

  return m_writer->addEntity(plane);
}