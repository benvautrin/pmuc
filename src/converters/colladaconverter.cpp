#include "colladaconverter.h"
#include "COLLADAConverter.h"

#include <iostream>
#include <set>

#include "../api/rvmcolorhelper.h"
#include "../api/rvmmeshhelper.h"

using namespace std;
using namespace COLLADASW;

class CCGroup {
    public:
        CCGroup(const std::string& name, const std::vector<float>& translation, const int& materialId) : m_name(name), m_translation(translation), m_material(materialId) {}

        void addGeometry(const string& name, vector<float>& matrix) { m_geometries.push_back(pair<string, vector<float> >(name, matrix)); }
        void addGroup(const CCGroup& group) { m_groups.push_back(group); }
        void addMetaData(const string& key, const string& value) { m_metaData.push_back(pair<string, string>(key, value)); }

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
    m_writer->openElement("asset");
    m_writer->openElement("contributor");
    m_writer->appendTextElement("author", user);
    m_writer->appendTextElement("authoring_tool", banner);
    m_writer->appendTextElement("comments", fileNote);
    m_writer->closeElement();
}

void COLLADAConverter::endHeader() {
    m_writer->closeElement(); // asset
}

void COLLADAConverter::startModel(const string& projectName, const string& name) {
    m_model = new CCModel(projectName, name);
    m_writer->openElement("library_geometries");
}

void COLLADAConverter::endModel() {
    m_writer->closeElement(); // library_geometries

    // Effects for materials
    m_writer->openElement("library_effects");
    for (set<int>::iterator it = m_model->materialIds().begin(); it != m_model->materialIds().end(); it++) {
        m_writer->openElement("effect");
        m_writer->appendAttribute("id", "E" + to_string((long long)*it));
        m_writer->openElement("profile_COMMON");
        m_writer->openElement("technique");
        m_writer->appendAttribute("sid", "COMMON");
        m_writer->openElement("lambert");
        m_writer->openElement("diffuse");
        m_writer->openElement("color");
        vector<float> color = RVMColorHelper::color(*it);
        m_writer->appendValues(color[0], color[1], color[2], 1);
        m_writer->closeElement(); // color
        m_writer->closeElement(); // diffuse
        m_writer->closeElement(); // lambert
        m_writer->closeElement(); // technique
        m_writer->closeElement(); // profile
        m_writer->closeElement(); // effect
    }
    m_writer->closeElement();

    // Materials
    m_writer->openElement("library_materials");
    for (set<int>::iterator it = m_model->materialIds().begin(); it != m_model->materialIds().end(); it++) {
        m_writer->openElement("material");
        m_writer->appendAttribute("id", "M" + to_string((long long)*it));
        m_writer->openElement("instance_effect");
        m_writer->appendAttribute("url", "#E" + to_string((long long)*it));
        m_writer->closeElement(); // instance_effect
        m_writer->closeElement(); // material
    }
    m_writer->closeElement(); // library_materials
}

void COLLADAConverter::startGroup(const std::string& name, const std::vector<float>& translation, const int& materialId) {
    CCGroup group(name, translation, materialId);
    m_model->groupStack().back()->addGroup(group);
    m_model->groupStack().push_back(m_model->groupStack().back());
    m_model->materialIds().insert(materialId);
}

void COLLADAConverter::endGroup() {
    m_model->groupStack().pop_back();
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

    m_writer->openElement("geometry");
    string gid = "G" + to_string((long long)m_model->geometryId()++);
    m_writer->appendAttribute("id", gid);
    m_writer->openElement("mesh");
    pair<vector<vector<float> >, vector<vector<int> > > c = RVMMeshHelper::makePyramid(xbottom, ybottom, xtop, ytop, height, xoffset, yoffset, m_maxSideSize, m_minSides);
    m_writer->openElement("source");
    m_writer->appendAttribute("id", gid + "C");
    m_writer->openElement("float_array");
    m_writer->appendAttribute("id", gid + "CA");
    vector<float> a;
    for (unsigned int i = 0; i < c.first.size(); i++)
        for (unsigned int j = 0; j < c.first[i].size(); j++)
            a.push_back(c.first[i][j]);
    m_writer->appendAttribute("count", a.size());
    m_writer->appendValues(a);
    m_writer->closeElement(); // float_array
    m_writer->openElement("technique_common");
    m_writer->openElement("accessor");
    m_writer->appendAttribute("count", a.size()/3);
    m_writer->appendAttribute("source", "#" + gid + "CA");
    m_writer->appendAttribute("stride", 3);
    m_writer->openElement("param");
    m_writer->appendAttribute("name", "X");
    m_writer->appendAttribute("type", "float");
    m_writer->closeElement(); // param
    m_writer->openElement("param");
    m_writer->appendAttribute("name", "Y");
    m_writer->appendAttribute("type", "float");
    m_writer->closeElement(); // param
    m_writer->openElement("param");
    m_writer->appendAttribute("name", "Z");
    m_writer->appendAttribute("type", "float");
    m_writer->closeElement(); // param
    m_writer->closeElement(); // accessor
    m_writer->closeElement(); // technique_common
    m_writer->closeElement(); // source
    m_writer->openElement("triangles");
    vector<unsigned long> na;
    for (unsigned int i = 0; i < c.second.size(); i++)
        for (unsigned int j = 0; j < c.second[i].size(); j++)
            na.push_back(c.second[i][j]);
    m_writer->appendAttribute("count", na.size()/3);
    m_writer->openElement("input");
    m_writer->appendAttribute("offset", 0);
    m_writer->appendAttribute("semantic", "POSITION");
    m_writer->appendAttribute("source", "#" + gid + "C");
    m_writer->closeElement(); // input
    m_writer->openElement("p");
    m_writer->appendValues(na);
    m_writer->closeElement(); // p
    m_writer->closeElement(); // triangles
    m_writer->closeElement(); // mesh
}

void COLLADAConverter::endPyramid() {
    m_writer->closeElement(); // geometry
}

void COLLADAConverter::startBox(const vector<float>& matrix,
                      const float& xlength,
                      const float& ylength,
                      const float& zlength) {
    cout << "startBox" << endl;
}

void COLLADAConverter::endBox() {
    cout << "endBox" << endl;
}

void COLLADAConverter::startRectangularTorus(const vector<float>& matrix,
                                   const float& rinside,
                                   const float& routside,
                                   const float& height,
                                   const float& angle) {
    cout << "startRectangularTorus" << endl;
}

void COLLADAConverter::endRectangularTorus() {
    cout << "endRectangularTorus" << endl;
}

void COLLADAConverter::startCircularTorus(const vector<float>& matrix,
                                const float& rinside,
                                const float& routside,
                                const float& angle) {
    cout << "startCircularTorus" << endl;
}

void COLLADAConverter::endCircularTorus() {
    cout << "endCircularTorus" << endl;
}

void COLLADAConverter::startEllipticalDish(const vector<float>& matrix,
                                 const float& diameter,
                                 const float& radius) {
    cout << "startEllipticalDish" << endl;
}

void COLLADAConverter::endEllipticalDish() {
    cout << "endEllipticalDish" << endl;
}

void COLLADAConverter::startSphericalDish(const vector<float>& matrix,
                                const float& diameter,
                                const float& height) {
    cout << "startSphericalDish" << endl;
}

void COLLADAConverter::endSphericalDish() {
    cout << "endSphericalDish" << endl;
}

void COLLADAConverter::startSnout(const vector<float>& matrix,
                        const float& dtop,
                        const float& dbottom,
                        const float& xoffset,
                        const float& yoffset,
                        const float& height,
                        const float& unknown1,
                        const float& unknown2,
                        const float& unknown3,
                        const float& unknown4) {
    cout << "startSnout" << endl;
}

void COLLADAConverter::endSnout() {
    cout << "endSnout" << endl;
}

void COLLADAConverter::startCylinder(const vector<float>& matrix,
                           const float& diameter,
                           const float& height) {
    cout << "startCylinder" << endl;
}

void COLLADAConverter::endCylinder() {
    cout << "endCylinder" << endl;
}

void COLLADAConverter::startSphere(const vector<float>& matrix,
                         const float& diameter) {
    cout << "startSphere" << endl;
}

void COLLADAConverter::endSphere() {
    cout << "endSphere" << endl;
}

void COLLADAConverter::startLine(const vector<float>& matrix,
                       const float& startx,
                       const float& endx) {
    cout << "startLine" << endl;
}

void COLLADAConverter::endLine() {
    cout << "endLine" << endl;
}

void COLLADAConverter::startFacetGroup(const vector<float>& matrix,
                             const vector<vector<vector<pair<Vector3F, Vector3F> > > >& vertexes) {
    cout << "startFacetGroup" << endl;
}

void COLLADAConverter::endFacetGroup() {
    cout << "endFacetGroup" << endl;
}
