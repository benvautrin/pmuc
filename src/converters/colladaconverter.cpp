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

#include "colladaconverter.h"

#include <COLLADASWAsset.h>
#include <COLLADASWInputList.h>
#include <COLLADASWLibraryGeometries.h>
#include <COLLADASWNode.h>
#include <COLLADASWPrimitves.h>
#include <COLLADASWSource.h>
#include <COLLADASWVertices.h>

#include <iostream>
#include <set>
#include <string>

#include "../api/rvmcolorhelper.h"
#include "../api/rvmmeshhelper.h"
#include "../common/stringutils.h"

using namespace std;
using namespace COLLADASW;

namespace colladaKeys {
enum keys {
  node,
  name,
  asset,
  keywords,
  translate,
  matrix,
  instance_geometry,
  url,
  bind_material,
  technique_common,
  instance_material,
  symbol,
  target,
  bind_vertex_input,
  semantic,
  input_semantic,
  input_set,
  contributor,
  author,
  authoring_tool,
  comments,
  up_axis,
  library_effects,
  effect,
  id,
  profile_COMMON,
  technique,
  sid,
  lambert,
  diffuse,
  color,
  library_materials,
  material,
  instance_effect,
  library_visual_scenes,
  visual_scene,
  scene,
  instance_visual_scene,
  geometry,
  library_geometries,
  mesh,
  source,
  float_array,
  count,
  accessor,
  stride,
  param,
  type,
  triangles,
  input,
  offset,
  p,
  ph,
  h,
  polygons,
  unit,
};
};

static String colladaKey[] = {"node",
                              "name",
                              "asset",
                              "keywords",
                              "translate",
                              "matrix",
                              "instance_geometry",
                              "url",
                              "bind_material",
                              "technique_common",
                              "instance_material",
                              "symbol",
                              "target",
                              "bind_vertex_input",
                              "semantic",
                              "input_semantic",
                              "input_set",
                              "contributor",
                              "author",
                              "authoring_tool",
                              "comments",
                              "up_axis",
                              "library_effects",
                              "effect",
                              "id",
                              "profile_COMMON",
                              "technique",
                              "sid",
                              "lambert",
                              "diffuse",
                              "color",
                              "library_materials",
                              "material",
                              "instance_effect",
                              "library_visual_scenes",
                              "visual_scene",
                              "scene",
                              "instance_visual_scene",
                              "geometry",
                              "library_geometries",
                              "mesh",
                              "source",
                              "float_array",
                              "count",
                              "accessor",
                              "stride",
                              "param",
                              "type",
                              "triangles",
                              "input",
                              "offset",
                              "p",
                              "ph",
                              "h",
                              "polygons",
                              "unit"};

class CCGroup {
 public:
  CCGroup(const std::string& name, const Vector3F& translation, const int& materialId)
      : m_name(name), m_translation(translation), m_material(materialId) {}

  void addGeometry(const string& name, const std::array<float, 12>& matrix) {
    m_geometries.push_back(pair<string, std::array<float, 12>>(name, matrix));
  }
  CCGroup& addGroup(const CCGroup& group) {
    m_groups.push_back(group);
    return m_groups.back();
  }
  void addMetaData(const string& key, const string& value) { m_metaData.push_back(pair<string, string>(key, value)); }

  std::string getNCName(std::string& name) {
    name.erase(remove_if(name.begin(), name.end(), [](char x) { return !isalnum(x) && !isspace(x); }), name.end());
    std::replace(name.begin(), name.end(), ' ', '-');
    return name;
  }

  void writeGroup(COLLADASW::StreamWriter* writer) {
    Node node(writer);
    node.setType(Node::NODE);
    node.setNodeName(getNCName(m_name));
    node.start();

    if (!m_metaData.empty()) {
      string data;
      for (unsigned int i = 0; i < m_metaData.size(); i++) {
        data += m_metaData[i].first + ": " + m_metaData[i].second + "\n";
      }
      Asset asset(writer);
      asset.setKeywords(data);
      asset.add();
    }

    node.addTranslate(m_translation[0], m_translation[1], m_translation[2]);

    for (unsigned int i = 0; i < m_geometries.size(); i++) {
      writer->openElement(colladaKey[colladaKeys::node]);
      writer->openElement(colladaKey[colladaKeys::matrix]);
      vector<float> m(16, 0.f);
      for (unsigned int j = 0; j < 4; j++) {
        for (unsigned int k = 0; k < 3; k++) {
          m[j + k * 4] = m_geometries[i].second[j * 3 + k];
        }
      }
      m[15] = 1.f;
      writer->appendValues(m);
      writer->closeElement();  // matrix
      writer->openElement(colladaKey[colladaKeys::instance_geometry]);
      writer->appendAttribute(colladaKey[colladaKeys::url], "#" + m_geometries[i].first);
      writer->openElement(colladaKey[colladaKeys::bind_material]);
      writer->openElement(colladaKey[colladaKeys::technique_common]);
      writer->openElement(colladaKey[colladaKeys::instance_material]);
      writer->appendAttribute(colladaKey[colladaKeys::symbol], "geometryMaterial");
      writer->appendAttribute(colladaKey[colladaKeys::target], "#M" + toString((long long)m_material));
      writer->openElement(colladaKey[colladaKeys::bind_vertex_input]);
      writer->appendAttribute(colladaKey[colladaKeys::semantic], "UVSET0");
      writer->appendAttribute(colladaKey[colladaKeys::input_semantic], "TEXCOORD");
      writer->appendAttribute(colladaKey[colladaKeys::input_set], 0);
      writer->closeElement();  // bind_vertex_input
      writer->closeElement();  // instance_material
      writer->closeElement();  // technique_common
      writer->closeElement();  // bind_material
      writer->closeElement();  // instance_geometry
      writer->closeElement();  // node
    }
    for (unsigned int i = 0; i < m_groups.size(); i++) {
      m_groups[i].writeGroup(writer);
    }
    node.end();
  }

 private:
  string m_name;
  Vector3F m_translation;
  int m_material;
  vector<pair<string, std::array<float, 12>>> m_geometries;
  vector<CCGroup> m_groups;
  vector<pair<string, string>> m_metaData;
};

class CCModel {
 public:
  CCModel(const string& projectName, const string& name)
      : m_projectName(projectName), m_name(name), m_baseGroup("", vector<float>(3, 0), 0), m_geometryId(0) {
    m_groupStack.push_back(&m_baseGroup);
  }

  CCGroup& group() { return m_baseGroup; }
  vector<CCGroup*>& groupStack() { return m_groupStack; }
  int& geometryId() { return m_geometryId; }
  set<int>& materialIds() { return m_materialIds; }

 private:
  string m_projectName;
  string m_name;
  CCGroup m_baseGroup;
  vector<CCGroup*> m_groupStack;
  int m_geometryId;
  set<int> m_materialIds;
};

COLLADAConverter::COLLADAConverter(const string& filename) : RVMReader(), m_model(0) {
  m_writer = new StreamWriter(NativeString(filename));
}

COLLADAConverter::~COLLADAConverter() {
  delete m_writer;
}

void COLLADAConverter::startDocument() {
  m_writer->startDocument();
}

void COLLADAConverter::endDocument() {
  m_writer->endDocument();
}

void COLLADAConverter::startHeader(const string& banner,
                                   const string& fileNote,
                                   const string& date,
                                   const string& user,
                                   const string& encoding) {
  Asset asset(m_writer);

  COLLADASW::Asset::Unit unit;
  unit.mName = "meter";
  unit.mMeter = 1.0;
  asset.setUnit(unit);

  asset.setUpAxisType(Asset::UpAxisType::Z_UP);

  asset.getContributor().mAuthor = user;
  asset.getContributor().mAuthoringTool = banner;
  asset.getContributor().mComments = fileNote;

  asset.add();
}

void COLLADAConverter::endHeader() {}

void COLLADAConverter::startModel(const string& projectName, const string& name) {
  m_model = new CCModel(projectName, name);
  m_writer->openElement(colladaKey[colladaKeys::library_geometries]);

  vector<float> origin;
  origin.push_back(0.);
  origin.push_back(0.);
  origin.push_back(0.);
  m_translations.push_back(origin);
}

void COLLADAConverter::endModel() {
  m_writer->closeElement();  // library_geometries

  // Effects for materials
  m_writer->openElement(colladaKey[colladaKeys::library_effects]);
  for (set<int>::iterator it = m_model->materialIds().begin(); it != m_model->materialIds().end(); it++) {
    m_writer->openElement(colladaKey[colladaKeys::effect]);
    m_writer->appendAttribute(colladaKey[colladaKeys::id], "E" + toString((long long)*it));
    m_writer->openElement(colladaKey[colladaKeys::profile_COMMON]);
    m_writer->openElement(colladaKey[colladaKeys::technique]);
    m_writer->appendAttribute(colladaKey[colladaKeys::sid], "COMMON");
    m_writer->openElement(colladaKey[colladaKeys::lambert]);
    m_writer->openElement(colladaKey[colladaKeys::diffuse]);
    m_writer->openElement(colladaKey[colladaKeys::color]);
    vector<float> color = RVMColorHelper::color(*it);
    m_writer->appendValues(color[0], color[1], color[2], 0.0f);
    m_writer->closeElement();  // color
    m_writer->closeElement();  // diffuse
    m_writer->closeElement();  // lambert
    m_writer->closeElement();  // technique
    m_writer->closeElement();  // profile
    m_writer->closeElement();  // effect
  }
  m_writer->closeElement();

  // Materials
  m_writer->openElement(colladaKey[colladaKeys::library_materials]);
  for (set<int>::iterator it = m_model->materialIds().begin(); it != m_model->materialIds().end(); it++) {
    m_writer->openElement(colladaKey[colladaKeys::material]);
    m_writer->appendAttribute(colladaKey[colladaKeys::id], "M" + toString((long long)*it));
    m_writer->openElement(colladaKey[colladaKeys::instance_effect]);
    m_writer->appendAttribute(colladaKey[colladaKeys::url], "#E" + toString((long long)*it));
    m_writer->closeElement();  // instance_effect
    m_writer->closeElement();  // material
  }
  m_writer->closeElement();  // library_materials

  // Visual scene
  m_writer->openElement(colladaKey[colladaKeys::library_visual_scenes]);
  m_writer->openElement(colladaKey[colladaKeys::visual_scene]);
  m_writer->appendAttribute(colladaKey[colladaKeys::id], "SCENE");
  m_model->group().writeGroup(m_writer);
  m_writer->closeElement();  // library_visual_scenes
  m_writer->closeElement();  // visual_scene

  // Scene
  m_writer->openElement(colladaKey[colladaKeys::scene]);
  m_writer->openElement(colladaKey[colladaKeys::instance_visual_scene]);
  m_writer->appendAttribute(colladaKey[colladaKeys::url], "#SCENE");
  m_writer->closeElement();  // instance_visual_scene
  m_writer->closeElement();  // scene
}

void COLLADAConverter::startGroup(const std::string& name, const Vector3F& translation, const int& materialId) {
  Vector3F t = translation - m_translations.back();
  CCGroup group(name, t, materialId);
  CCGroup* lastGroup = m_model->groupStack().back();
  m_model->groupStack().push_back(&lastGroup->addGroup(group));
  m_model->materialIds().insert(materialId);
  m_translations.push_back(translation);
}

void COLLADAConverter::endGroup() {
  m_model->groupStack().pop_back();
  m_translations.pop_back();
}

void COLLADAConverter::startMetaData() {}

void COLLADAConverter::endMetaData() {}

void COLLADAConverter::startMetaDataPair(const string& name, const string& value) {
  m_model->groupStack().back()->addMetaData(name, value);
}

void COLLADAConverter::endMetaDataPair() {}

void COLLADAConverter::createPyramid(const std::array<float, 12>& matrix, const Primitives::Pyramid& pyramid) {
  std::vector<float> params;
  params.push_back(Pyramid);
  params.push_back(pyramid.xbottom());
  params.push_back(pyramid.ybottom());
  params.push_back(pyramid.xtop());
  params.push_back(pyramid.ytop());
  params.push_back(pyramid.height());
  params.push_back(pyramid.xoffset());
  params.push_back(pyramid.yoffset());

  string gid = getInstanceName(params);
  if (gid.empty()) {
    gid = createGeometryId();
    writeMesh(gid, RVMMeshHelper2::makePyramid(pyramid, m_maxSideSize, m_minSides), "RVMPyramid");
    m_instanceMap.insert(std::make_pair(params, gid));
  }
  addGeometry(gid, matrix);
}

void COLLADAConverter::createBox(const std::array<float, 12>& matrix, const Primitives::Box& box) {
  std::vector<float> params;
  params.push_back(Box);
  params.push_back(box.len[0]);
  params.push_back(box.len[1]);
  params.push_back(box.len[2]);

  string gid = getInstanceName(params);
  if (gid.empty()) {
    gid = createGeometryId();
    writeMesh(gid, RVMMeshHelper2::makeBox(box, m_maxSideSize, m_minSides), "RVMBox");
    m_instanceMap.insert(std::make_pair(params, gid));
  }
  addGeometry(gid, matrix);
}

void COLLADAConverter::createRectangularTorus(const std::array<float, 12>& matrix,
                                              const Primitives::RectangularTorus& torus) {
  std::vector<float> params;
  params.push_back(RectangularTorus);
  params.push_back(torus.rinside());
  params.push_back(torus.routside());
  params.push_back(torus.height());
  params.push_back(torus.angle());

  string gid = getInstanceName(params);
  if (gid.empty()) {
    gid = createGeometryId();
    writeMesh(gid, RVMMeshHelper2::makeRectangularTorus(torus, m_maxSideSize, m_minSides), "RVMRectangularTorus");
    m_instanceMap.insert(std::make_pair(params, gid));
  }
  addGeometry(gid, matrix);
}

void COLLADAConverter::createCircularTorus(const std::array<float, 12>& matrix,
                                           const Primitives::CircularTorus& torus) {
  std::vector<float> params;
  params.push_back(CircularTorus);
  params.push_back(torus.offset());
  params.push_back(torus.radius());
  params.push_back(torus.angle());

  string gid = getInstanceName(params);
  if (gid.empty()) {
    gid = createGeometryId();
    auto sides = RVMMeshHelper2::infoCircularTorusNumSides(torus, m_maxSideSize, m_minSides);
    writeMesh(gid, RVMMeshHelper2::makeCircularTorus(torus, sides.first, sides.second), "RVMCircularTorus");
    m_instanceMap.insert(std::make_pair(params, gid));
  }
  addGeometry(gid, matrix);
}

void COLLADAConverter::createEllipticalDish(const std::array<float, 12>& matrix,
                                            const Primitives::EllipticalDish& dish) {
  std::vector<float> params;
  params.push_back(EllipticalDish);
  params.push_back(dish.diameter());
  params.push_back(dish.radius());

  string gid = getInstanceName(params);
  if (gid.empty()) {
    gid = createGeometryId();
    auto sideInfo = RVMMeshHelper2::infoEllipticalDishNumSides(dish, m_maxSideSize, m_minSides);
    writeMesh(gid, RVMMeshHelper2::makeEllipticalDish(dish, sideInfo.first, sideInfo.second), "RVMEllipticalDish");
    m_instanceMap.insert(std::make_pair(params, gid));
  }
  addGeometry(gid, matrix);
}

void COLLADAConverter::createSphericalDish(const std::array<float, 12>& matrix, const Primitives::SphericalDish& dish) {
  std::vector<float> params;
  params.push_back(SphericalDish);
  params.push_back(dish.diameter());
  params.push_back(dish.height());

  string gid = getInstanceName(params);
  if (gid.empty()) {
    gid = createGeometryId();
    writeMesh(gid, RVMMeshHelper2::makeSphericalDish(dish, m_maxSideSize, m_minSides), "RVMSphericalDish");
    m_instanceMap.insert(std::make_pair(params, gid));
  }
  addGeometry(gid, matrix);
}

void COLLADAConverter::createSnout(const std::array<float, 12>& matrix, const Primitives::Snout& snout) {
  std::vector<float> params;
  params.push_back(Snout);
  params.push_back(snout.dtop());
  params.push_back(snout.dbottom());
  params.push_back(snout.height());
  params.push_back(snout.xoffset());
  params.push_back(snout.yoffset());
  params.push_back(snout.xbshear());
  params.push_back(snout.ybshear());
  params.push_back(snout.xtshear());
  params.push_back(snout.ytshear());

  string gid = getInstanceName(params);
  if (gid.empty()) {
    gid = createGeometryId();
    writeMesh(gid,
              RVMMeshHelper2::makeSnout(snout, RVMMeshHelper2::infoSnoutNumSides(snout, m_maxSideSize, m_minSides)),
              "RVMSnout");
    m_instanceMap.insert(std::make_pair(params, gid));
  }
  addGeometry(gid, matrix);
}

void COLLADAConverter::createCylinder(const std::array<float, 12>& matrix, const Primitives::Cylinder& cylinder) {
  std::vector<float> params;
  params.push_back(Cylinder);
  params.push_back(cylinder.radius());
  params.push_back(cylinder.height());

  string gid = getInstanceName(params);
  if (gid.empty()) {
    gid = createGeometryId();
    writeMesh(gid,
              RVMMeshHelper2::makeCylinder(cylinder,
                                           RVMMeshHelper2::infoCylinderNumSides(cylinder, m_maxSideSize, m_minSides)),
              "RVMCylinder");
    m_instanceMap.insert(std::make_pair(params, gid));
  }
  addGeometry(gid, matrix);
}

void COLLADAConverter::createSphere(const std::array<float, 12>& matrix, const Primitives::Sphere& sphere) {
  std::vector<float> params;
  params.push_back(Sphere);
  params.push_back(sphere.diameter);

  string gid = getInstanceName(params);
  if (gid.empty()) {
    gid = createGeometryId();
    writeMesh(gid, RVMMeshHelper2::makeSphere(sphere, m_maxSideSize, m_minSides), "RVMSphere");
    m_instanceMap.insert(std::make_pair(params, gid));
  }
  addGeometry(gid, matrix);
}

void COLLADAConverter::createLine(const std::array<float, 12>& matrix, const float& thickness, const float& length) {
  std::vector<float> params;
  params.push_back(Line);
  params.push_back(length);  // thickness not taken into account yet

  string gid = getInstanceName(params);
  if (!gid.empty()) {
    m_model->groupStack().back()->addGeometry(gid, matrix);
    return;
  }
  m_writer->appendTextBlock("<!-- RVMLine -->");
  gid = createGeometryId();
  m_instanceMap.insert(std::make_pair(params, gid));

  m_writer->openElement(colladaKey[colladaKeys::geometry]);
  m_writer->appendAttribute(colladaKey[colladaKeys::id], gid);
  m_writer->openElement(colladaKey[colladaKeys::mesh]);

  FloatSourceF positionSource(m_writer);
  positionSource.setId(gid + "C");
  positionSource.setArrayId(gid + "CA");
  positionSource.setAccessorCount(2);
  positionSource.setAccessorStride(3);
  positionSource.getParameterNameList().push_back("X");
  positionSource.getParameterNameList().push_back("Y");
  positionSource.getParameterNameList().push_back("Z");
  positionSource.prepareToAppendValues();

  vector<float> a;
  a.push_back(-length / 2.0f);
  a.push_back(0);
  a.push_back(0);
  a.push_back(length / 2.0f);
  a.push_back(0);
  a.push_back(0);
  m_writer->appendValues(a);
  positionSource.finish();

  Vertices vertices(m_writer);
  vertices.setId(gid + "-vertices");
  vertices.getInputList().push_back(Input(InputSemantic::POSITION, URI("#" + positionSource.getId())));
  vertices.add();

  Lines lines(m_writer);
  lines.setCount((unsigned int)1);
  lines.setMaterial("geometryMaterial");
  lines.getInputList().push_back(Input(InputSemantic::VERTEX, URI("#" + vertices.getId()), 0));

  lines.prepareToAppendValues();
  lines.appendValues(0);
  lines.appendValues(1);
  lines.finish();
  //
  m_writer->closeElement();  // mesh
  m_writer->closeElement();  // geometry

  m_model->groupStack().back()->addGeometry(gid, matrix);
}

void COLLADAConverter::createFacetGroup(const std::array<float, 12>& matrix,
                                        const vector<vector<vector<Vertex>>>& vertexes) {
  Mesh meshData;
  string gid = createGeometryId();

  RVMMeshHelper2::tesselateFacetGroup(vertexes, &meshData);

  writeMesh(gid, meshData, "RVMFacetGroup");
  addGeometry(gid, matrix);
}

std::string COLLADAConverter::createGeometryId() {
  return "G" + toString((long long)m_model->geometryId()++);
}

std::string COLLADAConverter::getInstanceName(const std::vector<float>& params) {
  InstanceMap::iterator I = m_instanceMap.find(params);
  if (I != m_instanceMap.end()) {
    // cout << "Found instance: " << params << " " <<  gid <<endl;
    return (*I).second;
  }
  return "";
}

void COLLADAConverter::addGeometry(const std::string& name, const std::array<float, 12>& matrix) {
  std::array<float, 12> m = matrix;
  m[9] -= m_translations.back()[0];
  m[10] -= m_translations.back()[1];
  m[11] -= m_translations.back()[2];
  m_model->groupStack().back()->addGeometry(name, m);
}

void COLLADAConverter::writeMesh(const std::string& gid, const Mesh& mesh, const std::string comment) {
  bool hasNormals = mesh.normals.size() > 0;
  bool hasNormalIndex = mesh.normalIndex.size() > 0;

  if (!comment.empty()) {
    m_writer->appendTextBlock("<!-- " + comment + " -->");
  }

  m_writer->openElement(colladaKey[colladaKeys::geometry]);
  m_writer->appendAttribute(colladaKey[colladaKeys::id], gid);
  m_writer->openElement(colladaKey[colladaKeys::mesh]);

  FloatSourceF positionSource(m_writer);
  positionSource.setId(gid + "C");
  positionSource.setArrayId(gid + "CA");
  positionSource.setAccessorCount(static_cast<unsigned long>(mesh.positions.size()));
  positionSource.setAccessorStride(3);
  positionSource.getParameterNameList().push_back("X");
  positionSource.getParameterNameList().push_back("Y");
  positionSource.getParameterNameList().push_back("Z");
  positionSource.prepareToAppendValues();
  m_writer->appendValues(&mesh.positions.front()[0], mesh.positions.size() * 3);
  positionSource.finish();

  if (hasNormals) {
    FloatSourceF normalSource(m_writer);
    normalSource.setId(gid + "N");
    normalSource.setArrayId(gid + "NA");
    normalSource.setAccessorCount(static_cast<unsigned long>(mesh.normals.size()));
    normalSource.setAccessorStride(3);
    normalSource.getParameterNameList().push_back("X");
    normalSource.getParameterNameList().push_back("Y");
    normalSource.getParameterNameList().push_back("Z");
    normalSource.prepareToAppendValues();
    m_writer->appendValues(&mesh.normals.front()[0], mesh.normals.size() * 3);
    normalSource.finish();
  }

  Vertices vertices(m_writer);
  vertices.setId(gid + "-vertices");
  vertices.getInputList().push_back(Input(InputSemantic::POSITION, URI("#" + positionSource.getId())));
  vertices.add();

  Triangles t(m_writer);
  t.setCount(static_cast<unsigned long>(mesh.positionIndex.size() / 3));
  t.setMaterial("geometryMaterial");
  t.getInputList().push_back(Input(InputSemantic::VERTEX, URI("#" + vertices.getId()), 0));
  if (hasNormals) {
    t.getInputList().push_back(Input(InputSemantic::NORMAL, URI("#" + gid + "N"), 1));
  }
  t.prepareToAppendValues();
  if (hasNormalIndex) {
    for (size_t i = 0; i < mesh.positionIndex.size(); i++) {
      t.appendValues(mesh.positionIndex.at(i));
      t.appendValues(mesh.normalIndex.at(i));
    }
  } else {
    // We could use a single index, but some viewers (e.g. MeshLab) don't support it
    for (auto index : mesh.positionIndex) {
      t.appendValues(index, index);
    }
  }
  t.finish();

  m_writer->closeElement();  // mesh
  m_writer->closeElement();  // geometry
}
