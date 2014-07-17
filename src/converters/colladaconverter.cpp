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

#include <iostream>
#include <set>

#include "../api/rvmcolorhelper.h"
#include "../api/rvmmeshhelper.h"

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
	};
};

static String colladaKey[] = {
	"node",
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
};

class CCGroup {
    public:
        CCGroup(const std::string& name, const std::vector<float>& translation, const int& materialId) : m_name(name), m_translation(translation), m_material(materialId) {}

        void addGeometry(const string& name, const vector<float>& matrix) { m_geometries.push_back(pair<string, vector<float> >(name, matrix)); }
        CCGroup& addGroup(const CCGroup& group) { m_groups.push_back(group); return m_groups.back(); }
        void addMetaData(const string& key, const string& value) { m_metaData.push_back(pair<string, string>(key, value)); }

        void writeGroup(COLLADASW::StreamWriter* writer) {
            writer->openElement(colladaKey[colladaKeys::node]);
            writer->appendAttribute(colladaKey[colladaKeys::name], m_name);
            if (!m_metaData.empty()) {
                writer->openElement(colladaKey[colladaKeys::asset]);
                string data;
                for (unsigned int i = 0; i < m_metaData.size(); i++) {
                    data += m_metaData[i].first + ": " +  m_metaData[i].second + "\n";
                }
                writer->appendTextElement(colladaKey[colladaKeys::keywords], data);
                writer->closeElement(); // asset
            }
            writer->openElement(colladaKey[colladaKeys::translate]);
            writer->appendValues(m_translation[0], m_translation[1], m_translation[2]);
            writer->closeElement(); // translate
            for (unsigned int i = 0; i < m_geometries.size(); i++) {
                writer->openElement(colladaKey[colladaKeys::node]);
                writer->openElement(colladaKey[colladaKeys::matrix]);
                vector<float> m(16, 0.f);
                for (unsigned int j = 0; j < 4; j++) {
                    for (unsigned int k = 0; k < 3; k++) {
                        m[j+k*4] = m_geometries[i].second[j*3+k];
                    }
                }
                m[15] = 1.f;
                writer->appendValues(m);
                writer->closeElement(); // matrix
                writer->openElement(colladaKey[colladaKeys::instance_geometry]);
                writer->appendAttribute(colladaKey[colladaKeys::url], "#" + m_geometries[i].first);
                writer->openElement(colladaKey[colladaKeys::bind_material]);
                writer->openElement(colladaKey[colladaKeys::technique_common]);
                writer->openElement(colladaKey[colladaKeys::instance_material]);
                writer->appendAttribute(colladaKey[colladaKeys::symbol], "geometryMaterial");
                writer->appendAttribute(colladaKey[colladaKeys::target], "#M" + to_string((long long) m_material));
                writer->openElement(colladaKey[colladaKeys::bind_vertex_input]);
                writer->appendAttribute(colladaKey[colladaKeys::semantic], "UVSET0");
                writer->appendAttribute(colladaKey[colladaKeys::input_semantic], "TEXCOORD");
                writer->appendAttribute(colladaKey[colladaKeys::input_set], 0);
                writer->closeElement(); // bind_vertex_input
                writer->closeElement(); // instance_material
                writer->closeElement(); // technique_common
                writer->closeElement(); // bind_material
                writer->closeElement(); // instance_geometry
                writer->closeElement(); // node
            }
            for (unsigned int i = 0; i < m_groups.size(); i++) {
                m_groups[i].writeGroup(writer);
            }
            writer->closeElement(); // node
        }

    private:
        string m_name;
        vector<float> m_translation;
        int m_material;
        vector<pair<string, vector<float> > > m_geometries;
        vector<CCGroup> m_groups;
        vector<pair<string, string> > m_metaData;
};

class CCModel {
    public:
        CCModel(const string& projectName, const string& name) : m_projectName(projectName), m_name(name), m_baseGroup("", vector<float>(3,0), 0), m_geometryId(0) {
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

COLLADAConverter::COLLADAConverter(const string& filename)
    : RVMReader(),
      m_model(0) {
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

void COLLADAConverter::startHeader(const string& banner, const string& fileNote, const string& date, const string& user, const string& encoding) {
    m_writer->openElement(colladaKey[colladaKeys::asset]);
    m_writer->openElement(colladaKey[colladaKeys::contributor]);
    m_writer->appendTextElement(colladaKey[colladaKeys::author], user);
    m_writer->appendTextElement(colladaKey[colladaKeys::authoring_tool], banner);
    m_writer->appendTextElement(colladaKey[colladaKeys::comments], fileNote);
    m_writer->appendTextElement(colladaKey[colladaKeys::up_axis], "Z_UP");
    m_writer->closeElement();
}

void COLLADAConverter::endHeader() {
    m_writer->closeElement(); // asset
}

void COLLADAConverter::startModel(const string& projectName, const string& name) {
    m_model = new CCModel(projectName, name);
    m_writer->openElement(colladaKey[colladaKeys::library_geometries]);

    vector<float> origin;
    origin.push_back(0.); origin.push_back(0.); origin.push_back(0.);
    m_translations.push_back(origin);
}

void COLLADAConverter::endModel() {
    m_writer->closeElement(); // library_geometries

    // Effects for materials
    m_writer->openElement(colladaKey[colladaKeys::library_effects]);
    for (set<int>::iterator it = m_model->materialIds().begin(); it != m_model->materialIds().end(); it++) {
        m_writer->openElement(colladaKey[colladaKeys::effect]);
        m_writer->appendAttribute(colladaKey[colladaKeys::id], "E" + to_string((long long)*it));
        m_writer->openElement(colladaKey[colladaKeys::profile_COMMON]);
        m_writer->openElement(colladaKey[colladaKeys::technique]);
        m_writer->appendAttribute(colladaKey[colladaKeys::sid], "COMMON");
        m_writer->openElement(colladaKey[colladaKeys::lambert]);
        m_writer->openElement(colladaKey[colladaKeys::diffuse]);
        m_writer->openElement(colladaKey[colladaKeys::color]);
        vector<float> color = RVMColorHelper::color(*it);
        m_writer->appendValues(color[0], color[1], color[2], 0);
        m_writer->closeElement(); // color
        m_writer->closeElement(); // diffuse
        m_writer->closeElement(); // lambert
        m_writer->closeElement(); // technique
        m_writer->closeElement(); // profile
        m_writer->closeElement(); // effect
    }
    m_writer->closeElement();

    // Materials
    m_writer->openElement(colladaKey[colladaKeys::library_materials]);
    for (set<int>::iterator it = m_model->materialIds().begin(); it != m_model->materialIds().end(); it++) {
        m_writer->openElement(colladaKey[colladaKeys::material]);
        m_writer->appendAttribute(colladaKey[colladaKeys::id], "M" + to_string((long long)*it));
        m_writer->openElement(colladaKey[colladaKeys::instance_effect]);
        m_writer->appendAttribute(colladaKey[colladaKeys::url], "#E" + to_string((long long)*it));
        m_writer->closeElement(); // instance_effect
        m_writer->closeElement(); // material
    }
    m_writer->closeElement(); // library_materials

    // Visual scene
    m_writer->openElement(colladaKey[colladaKeys::library_visual_scenes]);
    m_writer->openElement(colladaKey[colladaKeys::visual_scene]);
    m_writer->appendAttribute(colladaKey[colladaKeys::id], "SCENE");
    m_model->group().writeGroup(m_writer);
    m_writer->closeElement(); // library_visual_scenes
    m_writer->closeElement(); // visual_scene

    // Scene
    m_writer->openElement(colladaKey[colladaKeys::scene]);
    m_writer->openElement(colladaKey[colladaKeys::instance_visual_scene]);
    m_writer->appendAttribute(colladaKey[colladaKeys::url], "#SCENE");
    m_writer->closeElement(); // instance_visual_scene
    m_writer->closeElement(); // scene
}

void COLLADAConverter::startGroup(const std::string& name, const std::vector<float>& translation, const int& materialId) {
    vector<float> t = translation;
    t[0] -= m_translations.back()[0];
    t[1] -= m_translations.back()[1];
    t[2] -= m_translations.back()[2];
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

void COLLADAConverter::startMetaData() {
}

void COLLADAConverter::endMetaData() {
}

void COLLADAConverter::startMetaDataPair(const string &name, const string &value) {
    m_model->groupStack().back()->addMetaData(name, value);
}

void COLLADAConverter::endMetaDataPair() {
}

void COLLADAConverter::startPyramid(const vector<float>& matrix,
                          const float& xbottom,
                          const float& ybottom,
                          const float& xtop,
                          const float& ytop,
                          const float& height,
                          const float& xoffset,
                          const float& yoffset) {

    writeGeometryWithoutNormals(matrix, RVMMeshHelper::makePyramid(xbottom, ybottom, xtop, ytop, height, xoffset, yoffset, m_maxSideSize, m_minSides));
}

void COLLADAConverter::endPyramid() {
    // in start.
}

void COLLADAConverter::startBox(const vector<float>& matrix,
                      const float& xlength,
                      const float& ylength,
                      const float& zlength) {

    writeGeometryWithoutNormals(matrix, RVMMeshHelper::makeBox(xlength, ylength, zlength, m_maxSideSize, m_minSides));
}

void COLLADAConverter::endBox() {
    // in start.
}

void COLLADAConverter::startRectangularTorus(const vector<float>& matrix,
                                   const float& rinside,
                                   const float& routside,
                                   const float& height,
                                   const float& angle) {
    writeGeometryWithNormals(matrix, RVMMeshHelper::makeRectangularTorus(rinside, routside, height, angle, m_maxSideSize, m_minSides));
}

void COLLADAConverter::endRectangularTorus() {
    // in start.
}

void COLLADAConverter::startCircularTorus(const vector<float>& matrix,
                                const float& rinside,
                                const float& routside,
                                const float& angle) {
    writeGeometryWithNormals(matrix, RVMMeshHelper::makeCircularTorus(rinside, routside, angle, m_maxSideSize, m_minSides));
}

void COLLADAConverter::endCircularTorus() {
    // in start.
}

void COLLADAConverter::startEllipticalDish(const vector<float>& matrix,
                                 const float& diameter,
                                 const float& radius) {
    writeGeometryWithNormals(matrix, RVMMeshHelper::makeEllipticalDish(diameter, radius, m_maxSideSize, m_minSides));
}

void COLLADAConverter::endEllipticalDish() {
    // in start.
}

void COLLADAConverter::startSphericalDish(const vector<float>& matrix,
                                const float& diameter,
                                const float& height) {
    writeGeometryWithNormals(matrix, RVMMeshHelper::makeSphericalDish(diameter, height, m_maxSideSize, m_minSides));
}

void COLLADAConverter::endSphericalDish() {
    // in start.
}

void COLLADAConverter::startSnout(const vector<float>& matrix,
                        const float& dtop,
                        const float& dbottom,
                        const float& height,
                        const float& xoffset,
                        const float& yoffset,
                        const float& unknown1,
                        const float& unknown2,
                        const float& unknown3,
                        const float& unknown4) {
    writeGeometryWithNormals(matrix, RVMMeshHelper::makeSnout(dtop, dbottom, height, xoffset, yoffset, m_maxSideSize, m_minSides));
}

void COLLADAConverter::endSnout() {
    // in start.
}

void COLLADAConverter::startCylinder(const vector<float>& matrix,
                           const float& radius,
                           const float& height) {
    writeGeometryWithNormals(matrix, RVMMeshHelper::makeCylinder(radius, height, m_maxSideSize, m_minSides));
}

void COLLADAConverter::endCylinder() {
    // in start.
}

void COLLADAConverter::startSphere(const vector<float>& matrix,
                         const float& diameter) {
    writeGeometryWithNormals(matrix, RVMMeshHelper::makeSphere(diameter, m_maxSideSize, m_minSides));
}

void COLLADAConverter::endSphere() {
    // in start.
}

void COLLADAConverter::startLine(const vector<float>& matrix,
                       const float& startx,
                       const float& endx) {

    m_writer->openElement(colladaKey[colladaKeys::geometry]);
    string gid = "G" + to_string((long long)m_model->geometryId()++);
    m_writer->appendAttribute(colladaKey[colladaKeys::id], gid);
    m_writer->openElement(colladaKey[colladaKeys::mesh]);
    m_writer->openElement(colladaKey[colladaKeys::source]);
    m_writer->appendAttribute(colladaKey[colladaKeys::id], gid + "C");
    m_writer->openElement(colladaKey[colladaKeys::float_array]);
    m_writer->appendAttribute(colladaKey[colladaKeys::id], gid + "CA");
    vector<float> a;
    a.push_back(startx); a.push_back(0); a.push_back(0); a.push_back(endx); a.push_back(0); a.push_back(0);
    m_writer->appendAttribute(colladaKey[colladaKeys::count], (unsigned int)a.size());
    m_writer->appendValues(a);
    m_writer->closeElement(); // float_array
    m_writer->openElement(colladaKey[colladaKeys::technique_common]);
    m_writer->openElement(colladaKey[colladaKeys::accessor]);
    m_writer->appendAttribute(colladaKey[colladaKeys::count], (unsigned int)(a.size()/3));
    m_writer->appendAttribute(colladaKey[colladaKeys::source], "#" + gid + "CA");
    m_writer->appendAttribute(colladaKey[colladaKeys::stride], 3);
    m_writer->openElement(colladaKey[colladaKeys::param]);
    m_writer->appendAttribute(colladaKey[colladaKeys::name], "X");
    m_writer->appendAttribute(colladaKey[colladaKeys::type], "float");
    m_writer->closeElement(); // param
    m_writer->openElement(colladaKey[colladaKeys::param]);
    m_writer->appendAttribute(colladaKey[colladaKeys::name], "Y");
    m_writer->appendAttribute(colladaKey[colladaKeys::type], "float");
    m_writer->closeElement(); // param
    m_writer->openElement(colladaKey[colladaKeys::param]);
    m_writer->appendAttribute(colladaKey[colladaKeys::name], "Z");
    m_writer->appendAttribute(colladaKey[colladaKeys::type], "float");
    m_writer->closeElement(); // param
    m_writer->closeElement(); // accessor
    m_writer->closeElement(); // technique_common
    m_writer->closeElement(); // source
    m_writer->openElement(colladaKey[colladaKeys::triangles]);
    vector<unsigned long> na;
    na.push_back(0); na.push_back(1);
    m_writer->appendAttribute(colladaKey[colladaKeys::count], (unsigned int)(na.size()/3));
    m_writer->appendAttribute(colladaKey[colladaKeys::material], "geometryMaterial");
    m_writer->openElement(colladaKey[colladaKeys::input]);
    m_writer->appendAttribute(colladaKey[colladaKeys::offset], 0);
    m_writer->appendAttribute(colladaKey[colladaKeys::semantic], "POSITION");
    m_writer->appendAttribute(colladaKey[colladaKeys::source], "#" + gid + "C");
    m_writer->closeElement(); // input
    m_writer->openElement(colladaKey[colladaKeys::p]);
    m_writer->appendValues(na);
    m_writer->closeElement(); // p
    m_writer->closeElement(); // triangles
    m_writer->closeElement(); // mesh
    m_writer->closeElement(); // geometry

    m_model->groupStack().back()->addGeometry(gid, matrix);
}

void COLLADAConverter::endLine() {
    // in start.
}

void COLLADAConverter::startFacetGroup(const vector<float>& matrix,
                             const vector<vector<vector<pair<Vector3F, Vector3F> > > >& vertexes) {

    vector<float> nc;
    vector<float> nn;
    unsigned long np = 0;
    for (unsigned int i = 0; i < vertexes.size(); i++) {
        for (unsigned int j = 0; j < vertexes[i].size(); j++) {
            for (unsigned int k = 0; k < vertexes[i][j].size(); k++) {
                nc.push_back(vertexes[i][j][k].first.x());
                nc.push_back(vertexes[i][j][k].first.y());
                nc.push_back(vertexes[i][j][k].first.z());
                nn.push_back(vertexes[i][j][k].second.x());
                nn.push_back(vertexes[i][j][k].second.y());
                nn.push_back(vertexes[i][j][k].second.z());
            }
            np++;
        }
    }
    m_writer->openElement(colladaKey[colladaKeys::geometry]);
    string gid = "G" + to_string((long long)m_model->geometryId()++);
    m_writer->appendAttribute(colladaKey[colladaKeys::id], gid);
    m_writer->openElement(colladaKey[colladaKeys::mesh]);
    // Write coordinates source
    m_writer->openElement(colladaKey[colladaKeys::source]);
    m_writer->appendAttribute(colladaKey[colladaKeys::id], gid + "C");
    m_writer->openElement(colladaKey[colladaKeys::float_array]);
    m_writer->appendAttribute(colladaKey[colladaKeys::id], gid + "CA");
    m_writer->appendAttribute(colladaKey[colladaKeys::count], (unsigned int)(nc.size()));
    m_writer->appendValues(nc);
    m_writer->closeElement(); // float_array
    m_writer->openElement(colladaKey[colladaKeys::technique_common]);
    m_writer->openElement(colladaKey[colladaKeys::accessor]);
    m_writer->appendAttribute(colladaKey[colladaKeys::count], (unsigned int)(nc.size()/3));
    m_writer->appendAttribute(colladaKey[colladaKeys::source], "#" + gid + "CA");
    m_writer->appendAttribute(colladaKey[colladaKeys::stride], 3);
    m_writer->openElement(colladaKey[colladaKeys::param]);
    m_writer->appendAttribute(colladaKey[colladaKeys::name], "X");
    m_writer->appendAttribute(colladaKey[colladaKeys::type], "float");
    m_writer->closeElement(); // param
    m_writer->openElement(colladaKey[colladaKeys::param]);
    m_writer->appendAttribute(colladaKey[colladaKeys::name], "Y");
    m_writer->appendAttribute(colladaKey[colladaKeys::type], "float");
    m_writer->closeElement(); // param
    m_writer->openElement(colladaKey[colladaKeys::param]);
    m_writer->appendAttribute(colladaKey[colladaKeys::name], "Z");
    m_writer->appendAttribute(colladaKey[colladaKeys::type], "float");
    m_writer->closeElement(); // param
    m_writer->closeElement(); // accessor
    m_writer->closeElement(); // technique_common
    m_writer->closeElement(); // source
    // Write normals source
    m_writer->openElement(colladaKey[colladaKeys::source]);
    m_writer->appendAttribute(colladaKey[colladaKeys::id], gid + "N");
    m_writer->openElement(colladaKey[colladaKeys::float_array]);
    m_writer->appendAttribute(colladaKey[colladaKeys::id], gid + "NA");
    m_writer->appendAttribute(colladaKey[colladaKeys::count], nn.size());
    m_writer->appendValues(nn);
    m_writer->closeElement(); // float_array
    m_writer->openElement(colladaKey[colladaKeys::technique_common]);
    m_writer->openElement(colladaKey[colladaKeys::accessor]);
    m_writer->appendAttribute(colladaKey[colladaKeys::count], nn.size()/3);
    m_writer->appendAttribute(colladaKey[colladaKeys::source], "#" + gid + "NA");
    m_writer->appendAttribute(colladaKey[colladaKeys::stride], 3);
    m_writer->openElement(colladaKey[colladaKeys::param]);
    m_writer->appendAttribute(colladaKey[colladaKeys::name], "X");
    m_writer->appendAttribute(colladaKey[colladaKeys::type], "float");
    m_writer->closeElement(); // param
    m_writer->openElement(colladaKey[colladaKeys::param]);
    m_writer->appendAttribute(colladaKey[colladaKeys::name], "Y");
    m_writer->appendAttribute(colladaKey[colladaKeys::type], "float");
    m_writer->closeElement(); // param
    m_writer->openElement(colladaKey[colladaKeys::param]);
    m_writer->appendAttribute(colladaKey[colladaKeys::name], "Z");
    m_writer->appendAttribute(colladaKey[colladaKeys::type], "float");
    m_writer->closeElement(); // param
    m_writer->closeElement(); // accessor
    m_writer->closeElement(); // technique_common
    m_writer->closeElement(); // source
    // Write triangles woth common index
    m_writer->openElement(colladaKey[colladaKeys::polygons]);
    m_writer->appendAttribute(colladaKey[colladaKeys::count], np);
    m_writer->appendAttribute(colladaKey[colladaKeys::material], "geometryMaterial");
    m_writer->openElement(colladaKey[colladaKeys::input]);
    m_writer->appendAttribute(colladaKey[colladaKeys::offset], 0);
    m_writer->appendAttribute(colladaKey[colladaKeys::semantic], "POSITION");
    m_writer->appendAttribute(colladaKey[colladaKeys::source], "#" + gid + "C");
    m_writer->closeElement(); // input
    m_writer->openElement(colladaKey[colladaKeys::input]);
    m_writer->appendAttribute(colladaKey[colladaKeys::offset], 0);
    m_writer->appendAttribute(colladaKey[colladaKeys::semantic], "NORMAL");
    m_writer->appendAttribute(colladaKey[colladaKeys::source], "#" + gid + "N");
    m_writer->closeElement(); // input
    unsigned int ci = 0;
    for (unsigned int i = 0; i < vertexes.size(); i++) {
        if (vertexes[i].size() > 1) {
            m_writer->openElement(colladaKey[colladaKeys::ph]);
        }
        for (unsigned int j = 0; j < vertexes[i].size(); j++) {
            if (j == 0) {
                m_writer->openElement(colladaKey[colladaKeys::p]);
            } else {
                m_writer->openElement(colladaKey[colladaKeys::h]);
            }
            vector<unsigned long> ni;
            for (unsigned int k = 0; k < vertexes[i][j].size(); k++) {
                ni.push_back(ci++);
            }
            m_writer->appendValues(ni);
            m_writer->closeElement(); // p
        }
        if (vertexes[i].size() > 1) {
            m_writer->closeElement(); // ph
        }
    }
    m_writer->closeElement(); // polygons
    m_writer->closeElement(); // mesh
    m_writer->closeElement(); // geometry

    vector<float> m = matrix;
    m[9] -= m_translations.back()[0];
    m[10] -= m_translations.back()[1];
    m[11] -= m_translations.back()[2];
    m_model->groupStack().back()->addGeometry(gid, m);
}

void COLLADAConverter::endFacetGroup() {
    // in start.
}

void COLLADAConverter::writeGeometryWithoutNormals(const vector<float>& matrix, const std::pair<std::vector<std::vector<float> >, std::vector<std::vector<int> > >& vertexes) {
    m_writer->openElement(colladaKey[colladaKeys::geometry]);
    string gid = "G" + to_string((long long)m_model->geometryId()++);
    m_writer->appendAttribute(colladaKey[colladaKeys::id], gid);
    m_writer->openElement(colladaKey[colladaKeys::mesh]);
    pair<vector<vector<float> >, vector<vector<int> > > c = vertexes;
    m_writer->openElement(colladaKey[colladaKeys::source]);
    m_writer->appendAttribute(colladaKey[colladaKeys::id], gid + "C");
    m_writer->openElement(colladaKey[colladaKeys::float_array]);
    m_writer->appendAttribute(colladaKey[colladaKeys::id], gid + "CA");
    vector<float> a;
    for (unsigned int i = 0; i < c.first.size(); i++)
        for (unsigned int j = 0; j < c.first[i].size(); j++)
            a.push_back(c.first[i][j]);
    m_writer->appendAttribute(colladaKey[colladaKeys::count], (unsigned int)(a.size()));
    m_writer->appendValues(a);
    m_writer->closeElement(); // float_array
    m_writer->openElement(colladaKey[colladaKeys::technique_common]);
    m_writer->openElement(colladaKey[colladaKeys::accessor]);
    m_writer->appendAttribute(colladaKey[colladaKeys::count], (unsigned int)(a.size()/3));
    m_writer->appendAttribute(colladaKey[colladaKeys::source], "#" + gid + "CA");
    m_writer->appendAttribute(colladaKey[colladaKeys::stride], 3);
    m_writer->openElement(colladaKey[colladaKeys::param]);
    m_writer->appendAttribute(colladaKey[colladaKeys::name], "X");
    m_writer->appendAttribute(colladaKey[colladaKeys::type], "float");
    m_writer->closeElement(); // param
    m_writer->openElement(colladaKey[colladaKeys::param]);
    m_writer->appendAttribute(colladaKey[colladaKeys::name], "Y");
    m_writer->appendAttribute(colladaKey[colladaKeys::type], "float");
    m_writer->closeElement(); // param
    m_writer->openElement(colladaKey[colladaKeys::param]);
    m_writer->appendAttribute(colladaKey[colladaKeys::name], "Z");
    m_writer->appendAttribute(colladaKey[colladaKeys::type], "float");
    m_writer->closeElement(); // param
    m_writer->closeElement(); // accessor
    m_writer->closeElement(); // technique_common
    m_writer->closeElement(); // source
    m_writer->openElement(colladaKey[colladaKeys::triangles]);
    vector<unsigned long> na;
    for (unsigned int i = 0; i < c.second.size(); i++)
        for (unsigned int j = 0; j < c.second[i].size(); j++)
            na.push_back(c.second[i][j]);
    m_writer->appendAttribute(colladaKey[colladaKeys::count], (unsigned int)(na.size()/3));
    m_writer->appendAttribute(colladaKey[colladaKeys::material], "geometryMaterial");
    m_writer->openElement(colladaKey[colladaKeys::input]);
    m_writer->appendAttribute(colladaKey[colladaKeys::offset], 0);
    m_writer->appendAttribute(colladaKey[colladaKeys::semantic], "POSITION");
    m_writer->appendAttribute(colladaKey[colladaKeys::source], "#" + gid + "C");
    m_writer->closeElement(); // input
    m_writer->openElement(colladaKey[colladaKeys::p]);
    m_writer->appendValues(na);
    m_writer->closeElement(); // p
    m_writer->closeElement(); // triangles
    m_writer->closeElement(); // mesh
    m_writer->closeElement(); // geometry

    vector<float> m = matrix;
    m[9] -= m_translations.back()[0];
    m[10] -= m_translations.back()[1];
    m[11] -= m_translations.back()[2];
    m_model->groupStack().back()->addGeometry(gid, m);
}

void COLLADAConverter::writeGeometryWithNormals(const std::vector<float>& matrix, const std::pair<std::pair<std::vector<std::vector<float> >, std::vector<std::vector<int> > >, std::pair<std::vector<std::vector<float> >, std::vector<std::vector<int> > > >& vertexes) {
    m_writer->openElement(colladaKey[colladaKeys::geometry]);
    string gid = "G" + to_string((long long)m_model->geometryId()++);
    m_writer->appendAttribute(colladaKey[colladaKeys::id], gid);
    m_writer->openElement(colladaKey[colladaKeys::mesh]);
    pair<vector<vector<float> >, vector<vector<int> > > c = vertexes.first;
    pair<vector<vector<float> >, vector<vector<int> > > n = vertexes.first;
    // Redo a flat list of coordinates for simplicity (collada doesn't support different indexes for coordinates and normals...)
    // Should be better.
    vector<float> nc;
    for (unsigned int i = 0; i < c.second.size(); i++) {
        for (unsigned int j = 0; j < c.second[i].size(); j++) {
            int fi = c.second[i][j];
            nc.push_back(c.first[fi][0]);
            nc.push_back(c.first[fi][1]);
            nc.push_back(c.first[fi][2]);
        }
    }
    vector<float> nn;
    for (unsigned int i = 0; i < n.second.size(); i++) {
        for (unsigned int j = 0; j < n.second[i].size(); j++) {
            int fi = n.second[i][j];
            nn.push_back(n.first[fi][0]);
            nn.push_back(n.first[fi][1]);
            nn.push_back(n.first[fi][2]);
        }
    }
    vector<unsigned long> ni;
    for (unsigned int i = 0; i < c.second.size() * 3; i++) {
        ni.push_back(i);
    }
    // Write coordinates source
    m_writer->openElement(colladaKey[colladaKeys::source]);
    m_writer->appendAttribute(colladaKey[colladaKeys::id], gid + "C");
    m_writer->openElement(colladaKey[colladaKeys::float_array]);
    m_writer->appendAttribute(colladaKey[colladaKeys::id], gid + "CA");
    m_writer->appendAttribute(colladaKey[colladaKeys::count], (unsigned int)(nc.size()));
    m_writer->appendValues(nc);
    m_writer->closeElement(); // float_array
    m_writer->openElement(colladaKey[colladaKeys::technique_common]);
    m_writer->openElement(colladaKey[colladaKeys::accessor]);
    m_writer->appendAttribute(colladaKey[colladaKeys::count], (unsigned int)(nc.size()/3));
    m_writer->appendAttribute(colladaKey[colladaKeys::source], "#" + gid + "CA");
    m_writer->appendAttribute(colladaKey[colladaKeys::stride], 3);
    m_writer->openElement(colladaKey[colladaKeys::param]);
    m_writer->appendAttribute(colladaKey[colladaKeys::name], "X");
    m_writer->appendAttribute(colladaKey[colladaKeys::type], "float");
    m_writer->closeElement(); // param
    m_writer->openElement(colladaKey[colladaKeys::param]);
    m_writer->appendAttribute(colladaKey[colladaKeys::name], "Y");
    m_writer->appendAttribute(colladaKey[colladaKeys::type], "float");
    m_writer->closeElement(); // param
    m_writer->openElement(colladaKey[colladaKeys::param]);
    m_writer->appendAttribute(colladaKey[colladaKeys::name], "Z");
    m_writer->appendAttribute(colladaKey[colladaKeys::type], "float");
    m_writer->closeElement(); // param
    m_writer->closeElement(); // accessor
    m_writer->closeElement(); // technique_common
    m_writer->closeElement(); // source
    // Write normals source
    m_writer->openElement(colladaKey[colladaKeys::source]);
    m_writer->appendAttribute(colladaKey[colladaKeys::id], gid + "N");
    m_writer->openElement(colladaKey[colladaKeys::float_array]);
    m_writer->appendAttribute(colladaKey[colladaKeys::id], gid + "NA");
    m_writer->appendAttribute(colladaKey[colladaKeys::count], (unsigned int)(nn.size()));
    m_writer->appendValues(nn);
    m_writer->closeElement(); // float_array
    m_writer->openElement(colladaKey[colladaKeys::technique_common]);
    m_writer->openElement(colladaKey[colladaKeys::accessor]);
    m_writer->appendAttribute(colladaKey[colladaKeys::count], (unsigned int)(nn.size()/3));
    m_writer->appendAttribute(colladaKey[colladaKeys::source], "#" + gid + "NA");
    m_writer->appendAttribute(colladaKey[colladaKeys::stride], 3);
    m_writer->openElement(colladaKey[colladaKeys::param]);
    m_writer->appendAttribute(colladaKey[colladaKeys::name], "X");
    m_writer->appendAttribute(colladaKey[colladaKeys::type], "float");
    m_writer->closeElement(); // param
    m_writer->openElement(colladaKey[colladaKeys::param]);
    m_writer->appendAttribute(colladaKey[colladaKeys::name], "Y");
    m_writer->appendAttribute(colladaKey[colladaKeys::type], "float");
    m_writer->closeElement(); // param
    m_writer->openElement(colladaKey[colladaKeys::param]);
    m_writer->appendAttribute(colladaKey[colladaKeys::name], "Z");
    m_writer->appendAttribute(colladaKey[colladaKeys::type], "float");
    m_writer->closeElement(); // param
    m_writer->closeElement(); // accessor
    m_writer->closeElement(); // technique_common
    m_writer->closeElement(); // source
    // Write triangles woth common index
    m_writer->openElement(colladaKey[colladaKeys::triangles]);
    m_writer->appendAttribute(colladaKey[colladaKeys::count], (unsigned int)(ni.size()/3));
    m_writer->appendAttribute(colladaKey[colladaKeys::material], "geometryMaterial");
    m_writer->openElement(colladaKey[colladaKeys::input]);
    m_writer->appendAttribute(colladaKey[colladaKeys::offset], 0);
    m_writer->appendAttribute(colladaKey[colladaKeys::semantic], "POSITION");
    m_writer->appendAttribute(colladaKey[colladaKeys::source], "#" + gid + "C");
    m_writer->closeElement(); // input
    m_writer->openElement(colladaKey[colladaKeys::input]);
    m_writer->appendAttribute(colladaKey[colladaKeys::offset], 0);
    m_writer->appendAttribute(colladaKey[colladaKeys::semantic], "NORMAL");
    m_writer->appendAttribute(colladaKey[colladaKeys::source], "#" + gid + "N");
    m_writer->closeElement(); // input
    m_writer->openElement(colladaKey[colladaKeys::p]);
    m_writer->appendValues(ni);
    m_writer->closeElement(); // p
    m_writer->closeElement(); // triangles
    m_writer->closeElement(); // mesh
    m_writer->closeElement(); // geometry

    vector<float> m = matrix;
    m[9] -= m_translations.back()[0];
    m[10] -= m_translations.back()[1];
    m[11] -= m_translations.back()[2];
    m_model->groupStack().back()->addGeometry(gid, m);
}
