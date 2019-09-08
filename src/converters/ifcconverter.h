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

#include "../api/rvmmeshhelper.h"
#include "../api/rvmreader.h"
#include "ifcwriter.h"

#define EIGEN_DONT_VECTORIZE
#define EIGEN_DISABLE_UNALIGNED_ARRAY_ASSERT

#include <Eigen/Core>
#include <map>
#include <stack>

typedef Eigen::Transform<float, 3, Eigen::Affine> Transform3f;

class IFCConverter : public RVMReader {
 public:
  IFCConverter(const std::string& filename, const std::string& schema);
  virtual ~IFCConverter();

  virtual void startDocument();
  virtual void endDocument();

  virtual void startHeader(const std::string& banner,
                           const std::string& fileNote,
                           const std::string& date,
                           const std::string& user,
                           const std::string& encoding);
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

  virtual void createFacetGroup(const std::array<float, 12>& matrix, const FGroup& vertexes);

 private:
  std::string m_filename;
  IFCStreamWriter* m_writer;

  IfcReference m_ownerHistory;
  IfcReference m_contextRef;
  IfcReference m_buildingRef;

  IfcEntity* m_project;

  std::stack<IfcEntity*> m_productStack;
  std::stack<IfcReferenceList> m_productMetaDataStack;
  std::stack<IfcReferenceList> m_productChildStack;
  std::stack<IfcReferenceList> m_productRepresentationStack;
  std::stack<IfcReference> m_placementStack;
  std::stack<int> m_currentMaterial;

  std::map<int, IfcReference> m_materials;
  std::map<int, IfcReference> m_styles;

  void createOwnerHistory(const std::string& name, const std::string& banner, int timeStamp);
  void createSlopedCylinder(const std::array<float, 12>& matrix, const Primitives::Snout& params);

  void initModel(const IfcReference projectRef);
  void createParentChildRelation(const IfcReference parent, const IfcReferenceList& children);
  void addStyleToItem(IfcReference item);
  void addRevolvedAreaSolidToShape(IfcReference profile, IfcReference axis, float angle, const Transform3f& transform);
  void writeMesh(const Mesh& mesh, const std::array<float, 12>& matrix);

  IfcReference createRepresentation();
  IfcReference createMaterial(int id);
  IfcReference createSurfaceStyle(int id);
  IfcReference createCoordinateSystem(const Transform3f& matrix, const Eigen::Vector3f& offset);
  IfcReference createClippingPlane(float zPos, const Eigen::Vector3f& n);
  IfcReference addCartesianPoint(float x, float y, float z, std::string entity = "IFCCARTESIANPOINT");
  IfcReference addCartesianPoint(float x, float y, std::string entity = "IFCCARTESIANPOINT");
  IfcReference createPropertySet(IfcReference relatedObject);
  IfcReference createPlacement(IfcReference parentPlacement);
};

#endif  // IFCCONVERTER_H
