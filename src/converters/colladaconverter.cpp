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

// Only compile if OpenCOLLADA is available
#ifdef OPENCOLLADASW_FOUND

#include "colladaconverter.h"

#include <COLLADASWPrimitves.h>
#include <COLLADASWInputList.h>
#include <COLLADASWSource.h>
#include <COLLADASWLibraryGeometries.h>

#include <iostream>
#include <set>

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
    "unit"
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
                writer->appendAttribute(colladaKey[colladaKeys::target], "#M" + toString((long long) m_material));
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
    m_writer->closeElement();
    m_writer->appendTextElement(colladaKey[colladaKeys::up_axis], "Z_UP");
    m_writer->openElement(colladaKey[colladaKeys::unit]);
    m_writer->appendAttribute("meter", "1.0");
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
        m_writer->appendAttribute(colladaKey[colladaKeys::id], "E" + toString((long long)*it));
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
        m_writer->appendAttribute(colladaKey[colladaKeys::id], "M" + toString((long long)*it));
        m_writer->openElement(colladaKey[colladaKeys::instance_effect]);
        m_writer->appendAttribute(colladaKey[colladaKeys::url], "#E" + toString((long long)*it));
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
    std::vector<float> params;
    params.push_back(Pyramid);
    params.push_back(xbottom);
    params.push_back(ybottom);
    params.push_back(xtop);
    params.push_back(ytop);
    params.push_back(height);
    params.push_back(xoffset);
    params.push_back(yoffset);

    string gid = getInstanceName(params);
    if(gid.empty()) {
       gid = createGeometryId();
       writeMesh(gid, RVMMeshHelper2::makePyramid(xbottom, ybottom, xtop, ytop, height, xoffset, yoffset, m_maxSideSize, m_minSides), "RVMPyramid");
       m_instanceMap.insert(std::make_pair(params, gid));
    }
    addGeometry(gid, matrix);
}

void COLLADAConverter::endPyramid() {
    // in start.
}

void COLLADAConverter::startBox(const vector<float>& matrix,
                      const float& xlength,
                      const float& ylength,
                      const float& zlength) {
    std::vector<float> params;
    params.push_back(Box);
    params.push_back(xlength);
    params.push_back(ylength);
    params.push_back(zlength);

    string gid = getInstanceName(params);
    if(gid.empty()) {
       gid = createGeometryId();
       writeMesh(gid, RVMMeshHelper2::makeBox(xlength, ylength, zlength, m_maxSideSize, m_minSides), "RVMBox");
       m_instanceMap.insert(std::make_pair(params, gid));
    }
    addGeometry(gid, matrix);

}

void COLLADAConverter::endBox() {
    // in start.
}

void COLLADAConverter::startRectangularTorus(const vector<float>& matrix,
                                   const float& rinside,
                                   const float& routside,
                                   const float& height,
                                   const float& angle) {
    std::vector<float> params;
    params.push_back(RectangularTorus);
    params.push_back(rinside);
    params.push_back(routside);
    params.push_back(height);
    params.push_back(angle);

    string gid = getInstanceName(params);
    if(gid.empty()) {
       gid = createGeometryId();
       writeMesh(gid, RVMMeshHelper2::makeRectangularTorus(rinside, routside, height, angle, m_maxSideSize, m_minSides), "RVMRectangularTorus");
       m_instanceMap.insert(std::make_pair(params, gid));
    }
    addGeometry(gid, matrix);

}

void COLLADAConverter::endRectangularTorus() {
    // in start.
}

void COLLADAConverter::startCircularTorus(const vector<float>& matrix,
                                const float& rinside,
                                const float& routside,
                                const float& angle) {
    std::vector<float> params;
    params.push_back(CircularTorus);
    params.push_back(rinside);
    params.push_back(routside);
    params.push_back(angle);

    string gid = getInstanceName(params);
    if(gid.empty()) {
       gid = createGeometryId();
       writeMesh(gid, RVMMeshHelper2::makeCircularTorus(rinside, routside, angle, m_maxSideSize, m_minSides), "RVMCircularTorus");
       m_instanceMap.insert(std::make_pair(params, gid));
    }
    addGeometry(gid, matrix);
}

void COLLADAConverter::endCircularTorus() {
    // in start.
}

void COLLADAConverter::startEllipticalDish(const vector<float>& matrix,
                                 const float& diameter,
                                 const float& radius) {

    std::vector<float> params;
    params.push_back(EllipticalDish);
    params.push_back(diameter);
    params.push_back(radius);

    string gid = getInstanceName(params);
    if(gid.empty()) {
       gid = createGeometryId();
       writeMesh(gid, RVMMeshHelper2::makeEllipticalDish(diameter, radius, m_maxSideSize, m_minSides), "RVMEllipticalDish");
       m_instanceMap.insert(std::make_pair(params, gid));
    }
    addGeometry(gid, matrix);
}

void COLLADAConverter::endEllipticalDish() {
    // in start.
}

void COLLADAConverter::startSphericalDish(const vector<float>& matrix,
                                const float& diameter,
                                const float& height) {
    std::vector<float> params;
    params.push_back(SphericalDish);
    params.push_back(diameter);
    params.push_back(height);

    string gid = getInstanceName(params);
    if(gid.empty()) {
       gid = createGeometryId();
       writeMesh(gid, RVMMeshHelper2::makeSphericalDish(diameter, height, m_maxSideSize, m_minSides), "RVMSphericalDish");
       m_instanceMap.insert(std::make_pair(params, gid));
    }
    addGeometry(gid, matrix);
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
    std::vector<float> params;
    params.push_back(Snout);
    params.push_back(dtop);
    params.push_back(dbottom);
    params.push_back(height);
    params.push_back(xoffset);
    params.push_back(yoffset);

    string gid = getInstanceName(params);
    if(gid.empty()) {
       gid = createGeometryId();
       writeMesh(gid, RVMMeshHelper2::makeSnout(dtop, dbottom, height, xoffset, yoffset, m_maxSideSize, m_minSides), "RVMSnout");
       m_instanceMap.insert(std::make_pair(params, gid));
    }
    addGeometry(gid, matrix);
}

void COLLADAConverter::endSnout() {
    // in start.
}

void COLLADAConverter::startCylinder(const vector<float>& matrix,
                           const float& radius,
                           const float& height) {
    std::vector<float> params;
    params.push_back(Cylinder);
    params.push_back(radius);
    params.push_back(height);

    string gid = getInstanceName(params);
    if(gid.empty()) {
       gid = createGeometryId();
       writeMesh(gid, RVMMeshHelper2::makeCylinder(radius, height, m_maxSideSize, m_minSides), "RVMCylinder");
       m_instanceMap.insert(std::make_pair(params, gid));
    }
    addGeometry(gid, matrix);
}

void COLLADAConverter::endCylinder() {
    // in start.
}

void COLLADAConverter::startSphere(const vector<float>& matrix,
                         const float& diameter) {
    std::vector<float> params;
    params.push_back(Sphere);
    params.push_back(diameter);

    string gid = getInstanceName(params);
    if(gid.empty()) {
       gid = createGeometryId();
       writeMesh(gid, RVMMeshHelper2::makeSphere(diameter, m_maxSideSize, m_minSides), "RVMSphere");
       m_instanceMap.insert(std::make_pair(params, gid));
    }
    addGeometry(gid, matrix);

}

void COLLADAConverter::endSphere() {
    // in start.
}

void COLLADAConverter::startLine(const vector<float>& matrix, const float& thickness, const float& length) {
    std::vector<float> params;
    params.push_back(Line);
    params.push_back(length); // thickness not taken into account yet

    string gid = getInstanceName(params);
    if(!gid.empty()) {
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
    positionSource.setId(gid+"C");
    positionSource.setArrayId(gid+"CA");
    positionSource.setAccessorCount(2);
    positionSource.setAccessorStride(3);
    positionSource.getParameterNameList().push_back("X");
    positionSource.getParameterNameList().push_back("Y");
    positionSource.getParameterNameList().push_back("Z");
    positionSource.prepareToAppendValues();


    vector<float> a;
    a.push_back(-length/2.0f); a.push_back(0); a.push_back(0); a.push_back(length/2.0f); a.push_back(0); a.push_back(0);
    m_writer->appendValues(a);
    positionSource.finish();

    Lines lines(m_writer);
    lines.setCount((unsigned int) 1);
    lines.setMaterial("geometryMaterial");
    lines.getInputList().push_back(Input(InputSemantic::POSITION, URI("#" + positionSource.getId()), 0));

    lines.prepareToAppendValues();
    lines.appendValues(0);
    lines.appendValues(1);
    lines.finish();
    //
    m_writer->closeElement(); // mesh
    m_writer->closeElement(); // geometry

    m_model->groupStack().back()->addGeometry(gid, matrix);
}

void COLLADAConverter::endLine() {
    // in start.
}

void COLLADAConverter::startFacetGroup(const vector<float>& matrix,
                             const vector<vector<vector<Vertex> > >& vertexes) {
    Mesh meshData;
    string gid = createGeometryId();

    RVMMeshHelper2::tesselateFacetGroup(vertexes, &meshData);

    writeMesh(gid, meshData, "RVMFacetGroup");
    addGeometry(gid, matrix);
}

void COLLADAConverter::endFacetGroup() {
    // in start.
}

std::string COLLADAConverter::createGeometryId() {
        return "G" + toString((long long)m_model->geometryId()++);
}

std::string COLLADAConverter::getInstanceName(const std::vector<float> &params) {
    InstanceMap::iterator I = m_instanceMap.find(params);
    if(I != m_instanceMap.end()) {
       //cout << "Found instance: " << params << " " <<  gid <<endl;
       return (*I).second;
    }
    return "";
}


void COLLADAConverter::addGeometry(const std::string &name, const vector<float>& matrix) {
    vector<float> m = matrix;
    m[9] -= m_translations.back()[0];
    m[10] -= m_translations.back()[1];
    m[11] -= m_translations.back()[2];
    m_model->groupStack().back()->addGeometry(name, m);
}

void COLLADAConverter::writeMesh(const std::string &gid, const Mesh &mesh, const std::string comment) {
	bool hasNormals = mesh.normals.size() > 0;
	bool hasNormalIndex = mesh.normalIndex.size() > 0;

    if(!comment.empty()) {
        m_writer->appendTextBlock("<!-- " + comment + " -->");
    }

	m_writer->openElement(colladaKey[colladaKeys::geometry]);
    m_writer->appendAttribute(colladaKey[colladaKeys::id], gid);
    m_writer->openElement(colladaKey[colladaKeys::mesh]);

	FloatSourceF positionSource(m_writer);
	positionSource.setId(gid+"C");
	positionSource.setArrayId(gid+"CA");
	positionSource.setAccessorCount(mesh.positions.size());
	positionSource.setAccessorStride(3);
	positionSource.getParameterNameList().push_back("X");
	positionSource.getParameterNameList().push_back("Y");
	positionSource.getParameterNameList().push_back("Z");
	positionSource.prepareToAppendValues();
	m_writer->appendValues(&mesh.positions.front()[0], mesh.positions.size() * 3);
    positionSource.finish();

	if(hasNormals) {
		FloatSourceF normalSource(m_writer);
		normalSource.setId(gid+"N");
		normalSource.setArrayId(gid+"NA");
		normalSource.setAccessorCount(mesh.normals.size());
		normalSource.setAccessorStride(3);
		normalSource.getParameterNameList().push_back("X");
		normalSource.getParameterNameList().push_back("Y");
		normalSource.getParameterNameList().push_back("Z");
		normalSource.prepareToAppendValues();
		m_writer->appendValues(&mesh.normals.front()[0], mesh.normals.size() * 3);
		normalSource.finish();
	}

	Triangles t(m_writer);
	t.setCount(mesh.positionIndex.size() / 3);
	t.setMaterial("geometryMaterial");
	t.getInputList().push_back(Input(InputSemantic::POSITION, URI("#" + gid + "C"), 0));
	if(hasNormals) {
		if(hasNormalIndex) {
			t.getInputList().push_back(Input(InputSemantic::NORMAL, URI("#" + gid + "N"), 1));
		} else {
			t.getInputList().push_back(Input(InputSemantic::NORMAL, URI("#" + gid + "N"), 0));
		}
	}
	t.prepareToAppendValues();
	if(hasNormalIndex) {
		for(size_t i = 0; i < mesh.positionIndex.size(); i++) {
			t.appendValues(mesh.positionIndex.at(i));
			t.appendValues(mesh.normalIndex.at(i));
		}
	} else {
		// Just use the position index
		t.appendValues(mesh.positionIndex);
	}
	t.finish();

	m_writer->closeElement(); // mesh
    m_writer->closeElement(); // geometry
}

#endif // OPENCOLLADASW_FOUND
