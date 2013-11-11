#include "dslwriter.h"

#include <stdlib.h>

#ifdef _MSC_VER
#include <stdio.h>
#else
#include <sys/ioctl.h>
#endif

using namespace std;

DSLWriter::DSLWriter() {
}

bool DSLWriter::open(const string& filename){
    fp = fopen(filename.data(), "w");
    return fp == 0;
}

void DSLWriter::close(){
    fclose(fp);
}

void DSLWriter::writeTranslation(const string& newId, const string& objectId, float x, float y, float z){
    fprintf(fp, "%s = translate_shape(%s, Vector(%f, %f, %f))\n", newId.data(), objectId.data(), x, y, z);
}

void DSLWriter::writeRotation(const string& newId, const string& objectId, float x, float y, float z){
    fprintf(fp, "%s = rotate_shape_3_axis(%s, %f, %f, %f)\n", newId.data(), objectId.data(), x, y, z);
}

void DSLWriter::writeSphere(const string& id, float radius){
    fprintf(fp, "%s = make_sphere(%f)\n", id.data(), radius);
}

void DSLWriter::writeBox(const string& id, float lx, float ly, float lz){
    fprintf(fp, "%s = make_box(%f, %f, %f)\n", id.data(), lx, ly, lz);
}

void DSLWriter::writeCone(const string& id, float major_radius, float minor_radius, float height){
    fprintf(fp, "%s = make_cone(%f, %f, %f)\n", id.data(), major_radius, minor_radius, height);
}

void DSLWriter::writeSnout(const string& id, float major_radius, float minor_radius, float xoff, float yoff, float height){
    fprintf(fp, "%s = make_snout(%f, %f, %f, %f, %f)\n", id.data(), major_radius, minor_radius, xoff, yoff, height);
}

void DSLWriter::writeWedge(const string& id, float x, float y, float z, float angle){
    fprintf(fp, "%s = make_wedge(%f, %f, %f, %f)\n", id.data(), x, y, z, angle);
}

void DSLWriter::writeCircularTorus(const string& id, float r_outside, float r_inside, float angle){
    fprintf(fp, "%s = make_torus(%f, %f, %f)\n", id.data(), r_outside, r_inside, angle);
}

void DSLWriter::writeRectangularTorus(const string& id, float r_outside, float r_inside, float height, float angle){
    fprintf(fp, "%s = make_rectangular_torus(%f, %f, %f, %f)\n", id.data(), r_outside, r_inside, height, angle);
}

void DSLWriter::writeCylinder(const string& id, float radius, float height){
    fprintf(fp, "%s = make_cylinder(%f, %f)\n", id.data(), radius, height);
}

void DSLWriter::writeDish(const string& id, float height, float diameter, float angle){
    fprintf(fp, "%s = make_dish(%f, %f, %f)\n", id.data(), height, diameter, angle);
}

void DSLWriter::writePyramid(const string& id, float lx_bottom, float ly_bottom, float lx_top, float ly_top, float height, float xoff, float yoff){
    fprintf(fp, "%s = make_pyramid(%f, %f, %f, %f, %f, %f, %f)\n", id.data(), lx_bottom, ly_bottom, lx_top, ly_top, height, xoff, yoff);
}

void DSLWriter::writeNozzle(const string& id, float height, float r_inside, float r_outside, float nozzle_height, float nozzle_radius){
    fprintf(fp, "%s = make_nozzle(%f, %f, %f, %f, %f)\n", id.data(), height, r_inside, r_outside, nozzle_height, nozzle_radius);
}

void DSLWriter::writeLine(const string& id, float x1, float y1, float z1, float x2, float y2, float z2){
    fprintf(fp, "%s_1 = make_point(%f, %f, %f)\n%s_2 = make_point(%f, %f, %f)\n%s = make_line(%s_1, %s_2)\n",
            id.data(), x1, y1, z1, id.data(), x2, y2, z2, id.data(), id.data(), id.data());
}

void DSLWriter::writeGroup(const string& id, const vector<string>& children){
    fprintf(fp, "%s = ", id.data());
    for (unsigned int i = 0; i < children.size(); i++) {
        fprintf(fp, "%s %s", i == 0 ? "" : " +", children[i].data());
    }
    fprintf(fp, "\n");
}

