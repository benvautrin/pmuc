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

#include "rvmmeshhelper.h"


#ifdef _WIN32
  #include "windows.h"
#endif

#include <GL/gl.h>
#include <GL/glu.h>

#ifndef WIN32
#  define CALLBACK
#endif /* !WIN32 */

#include <iostream>
#include <algorithm>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

#ifdef WIN32
#undef min
#undef max
#endif

using namespace std;

RVMMeshHelper2::RVMMeshHelper2()
{
}

static const float cube_positions[] = {
   -1.0f,-1.0f,-1.0f,  -1.0f, 1.0f,-1.0f,   1.0f, 1.0f,-1.0f,   1.0f,-1.0f,-1.0f,
    -1.0f,-1.0f, 1.0f,  -1.0f, 1.0f, 1.0f,   1.0f, 1.0f, 1.0f,   1.0f,-1.0f, 1.0f,
    -1.0f,-1.0f,-1.0f,  -1.0f,-1.0f, 1.0f,  -1.0f, 1.0f, 1.0f,  -1.0f, 1.0f,-1.0f,
    1.0f,-1.0f,-1.0f,   1.0f,-1.0f, 1.0f,   1.0f, 1.0f, 1.0f,   1.0f, 1.0f,-1.0f,
    -1.0f, 1.0f,-1.0f,  -1.0f, 1.0f, 1.0f,   1.0f, 1.0f, 1.0f,   1.0f, 1.0f,-1.0f,
    -1.0f,-1.0f,-1.0f,  -1.0f,-1.0f, 1.0f,   1.0f,-1.0f, 1.0f,   1.0f,-1.0f,-1.0f
};

static const float cube_normals[] = {
   0.0f, 0.0f,-1.0f,
   0.0f, 0.0f, 1.0f,
  -1.0f, 0.0f, 0.0f,
   1.0f, 0.0f, 0.0f,
   0.0f, 1.0f, 0.0f,
   0.0f, -1.0f, 0.0f,
};


static const unsigned long cube_index[] = {
    0,1,2, 2,3,0,
    4,7,5, 5,7,6,
    8,9,10, 10,11,8,
    12,14,13, 14,12,15,
    16,17,18, 18,19,16,
    20,22,21, 22,20,23
};

void CALLBACK tessVertexData(void * vertex_data, void * polygon_data) {
    Mesh* userData = (Mesh*) polygon_data;
    userData->positionIndex.push_back((unsigned long)vertex_data);
}
void CALLBACK tessEdgeFlag(GLboolean flag, void * polygon_data) {
}

void CALLBACK tessError(GLenum err) {
    cout << "error: " << gluErrorString(err) << endl;
}

void CALLBACK tessCombineData(GLdouble newVert[3], GLdouble *neighbourVert[4], GLfloat neighborWeight[4], void **outData, void * polygon_data)
{
    Mesh* userData = (Mesh*) polygon_data;
    unsigned long index = static_cast<unsigned long>( userData->positions.size() );
    userData->positions.push_back(Vector3F(float(newVert[0]), float(newVert[1]), float(newVert[2])));
    userData->normals.push_back(Vector3F(0.0f, 1.0f, 0.0f));
    *outData = (void*)index;
};

const Mesh RVMMeshHelper2::makeBox(const Primitives::Box& box, const float &maxSideSize, const int &minSides)
{
    vector<Vector3F> points;
    vector<Vector3F> normals;
    for (int i = 0; i < 24; i++) {
        Vector3F point, normal;
        point[0] = cube_positions[i * 3] * box.len[0] * 0.5f;
        point[1] = cube_positions[i * 3 + 1] * box.len[1] * 0.5f;
        point[2] = cube_positions[i * 3 + 2] * box.len[2] * 0.5f;
        normal[0] = cube_normals[i/4*3];
        normal[1] = cube_normals[i/4*3+1];
        normal[2] = cube_normals[i/4*3+2];
        points.push_back(point);
        normals.push_back(normal);
    }

    // Copy the index of the box
    vector<unsigned long> index (cube_index, cube_index + sizeof(cube_index) / sizeof(cube_index[0]) );

    Mesh result;
    result.positions = points;
    result.positionIndex = index;
    result.normals = normals;
    return result;
}

const Mesh RVMMeshHelper2::makeSphere(const Primitives::Sphere &sphere, const float& maxSideSize, const int& minSides)
{
    const float radius = sphere.diamater / 2.0f;

    // Init sphere
    int sides = max(8, minSides);
    vector<Vector3F> positions;
    vector<Vector3F> normals;


    for (int x = 0; x<= sides; x++) {
        float theta = float((x * M_PI) / (float) sides);
        float sinTheta = sinf(theta);
        float cosTheta = cosf(theta);

        for (int y = 0; y <= sides; y++) {
            float phi = float((y * 2.0 * M_PI) / float(sides));
            float sinPhi = sin(phi);
            float cosPhi = cos(phi);

            Vector3F v;
            v[0] = -cosPhi * sinTheta;
            v[1] = -cosTheta;
            v[2] = -sinPhi * sinTheta;

            normals.push_back(v);
            positions.push_back(v * radius);
        }
    }

    vector<unsigned long> index;

    for (int i = 0; i < sides; i++) {
        for (int j = 0; j < sides; j++) {
            int first = (i * (sides + 1)) + j;
            int second = first + sides + 1;

            index.push_back(first);
            index.push_back(second);
            index.push_back(first + 1);

            index.push_back(second);
            index.push_back(second + 1);
            index.push_back(first + 1);
        }
    }

    Mesh result;
    result.positions = positions;
    result.positionIndex = index;
    result.normals = normals;
    return result;
}

const Mesh RVMMeshHelper2::makeRectangularTorus(const Primitives::RectangularTorus& rt, const float& maxSideSize, const int& minSides)
{
    vector<unsigned long> index;
    vector<Vector3F> points;

    vector<unsigned long> normalindex;
    vector<Vector3F> vectors;

    int sides = int(rt.angle() * rt.routside() / maxSideSize);
    if (sides < minSides) {
        sides = minSides;
    }

    // Vertexes and normals
    Vector3F v;
    vectors.push_back(Vector3F(0,0,-1));
    vectors.push_back(Vector3F(0,0,1));

    for (int i = 0; i < sides+1; i++) {
        float c = cos(rt.angle() / sides * i);
        float s = sin(rt.angle() / sides * i);
        v[0] = rt.rinside() * c; v[1] = rt.rinside() * s; v[2] = -rt.height() / 2.f;
        points.push_back(v);
        v[0] = rt.routside() * c; v[1] = rt.routside() * s; v[2]= -rt.height() / 2.f;
        points.push_back(v);
        v[0] = rt.routside() * c; v[1] = rt.routside() * s; v[2] = rt.height() / 2.f;
        points.push_back(v);
        v[0] = rt.rinside() * c; v[1] = rt.rinside() * s; v[2] = rt.height() / 2.f;
        points.push_back(v);
        vectors.push_back(Vector3F(c,s,0));
        vectors.push_back(Vector3F(-c,-s,0));
    }

    // Sides
    vector<int> ni(3, 0);
    for (int i = 0; i < sides; i++) {
        for (int j = 0; j < 4; j++) {
            index.push_back(i*4+j);
            index.push_back(i*4+4+j);
            index.push_back(j < 3 ? i*4+1+j : i*4);

            for (int k = 0; k < 3; k++) {
                    switch(j) {
                    case 0:
                        normalindex.push_back(0);
                        break;
                    case 1:
                        normalindex.push_back(k == 1 ? i*2+4 : i*2+2);
                        break;
                    case 2:
                        normalindex.push_back(1);
                        break;
                    case 3:
                        normalindex.push_back(k == 1 ? i*2+5 : i*2+3);
                        break;
                }
            }

            index.push_back(i*4+4+j);
            index.push_back(j < 3 ? i*4+5+j : i*4+4);
            index.push_back(j < 3 ? i*4+1+j : i*4);

            for (int k = 0; k < 3; k++) {
                    switch(j) {
                    case 0:
                        normalindex.push_back(0);
                        break;
                    case 1:
                        normalindex.push_back((k == 0 || k == 1) ? i*2+4 : i*2+2);
                        break;
                    case 2:
                        normalindex.push_back(1);
                        break;
                    case 3:
                        normalindex.push_back((k == 0 || k == 1) ? i*2+5 : i*2+3);
                        break;
                }
            }

        }
    }

    // Caps
    // - Caps normals
    const unsigned long nci = static_cast<unsigned long>( vectors.size() );
    vectors.push_back(Vector3F(0,-1,0));
    float c = cos(rt.angle());
    float s = sin(rt.angle());
    vectors.push_back(Vector3F(-s,c,0));
    // - Caps indexes
    index.push_back(0);
    index.push_back(1);
    index.push_back(2);

    normalindex.push_back(nci);
    normalindex.push_back(nci);
    normalindex.push_back(nci);

    index.push_back(0);
    index.push_back(2);
    index.push_back(3);

    normalindex.push_back(nci);
    normalindex.push_back(nci);
    normalindex.push_back(nci);

    //
    index.push_back(sides*4);
    index.push_back(sides*4+2);
    index.push_back(sides*4+1);

    normalindex.push_back(nci+1);
    normalindex.push_back(nci+1);
    normalindex.push_back(nci+1);

    index.push_back(sides*4);
    index.push_back(sides*4+3);
    index.push_back(sides*4+2);

    normalindex.push_back(nci+1);
    normalindex.push_back(nci+1);
    normalindex.push_back(nci+1);

    Mesh result;
    result.positions = points;
    result.positionIndex = index;
    result.normals= vectors;
    result.normalIndex = normalindex;

    return result;
}

std::pair<unsigned long, unsigned long> RVMMeshHelper2::infoCircularTorusNumSides(const Primitives::CircularTorus& cTorus, float maxSideSize, unsigned long minSides)
{
    unsigned long tsides = std::max(minSides, static_cast<unsigned long>(cTorus.angle() * cTorus.rinside() / maxSideSize) );
    unsigned long csides = std::max(minSides, static_cast<unsigned long>(2 * M_PI * cTorus.routside() / maxSideSize) );

    return std::make_pair(tsides, csides);
}

const Mesh RVMMeshHelper2::makeCircularTorus(const Primitives::CircularTorus& cTorus, unsigned long tsides, unsigned long csides)
{
    vector<unsigned long> index;
    vector<Vector3F> points;
    vector<unsigned long> normalindex;
    vector<Vector3F> vectors;

    // Vertexes and normals
    float rcenter = cTorus.routside();
    float center = cTorus.rinside();
    Vector3F v;
    Vector3F n;
    const float da = cTorus.angle() / static_cast<float>(tsides);
    const float da2 = 2.0f * M_PI / static_cast<float>(csides);

    for (unsigned long i = 0; i < tsides+1; i++)
    {
        const float a = da * static_cast<float>(i);
        const float c = cos(a);
        const float s = sin(a);

        for (unsigned long j = 0; j < csides; j++)
        {
            const float a2 = da2 * static_cast<float>(j);
            const float C = cos(a2);
            const float S = sin(a2);

            v[0] = (rcenter * C + center) * c;
            v[1] = (rcenter * C + center) * s;
            v[2] = rcenter * S;
            points.push_back(v);

            n[0] = C*c;
            n[1] = C*s;
            n[2] = S;
            n.normalize();
            vectors.push_back(n);
        }
    }

    // Sides
    for (unsigned long i = 0; i < tsides; i++)
        for (unsigned long j = 0; j < csides; j++)
        {
            unsigned long pi = i*csides + j;
            index.push_back(pi);
            normalindex.push_back(pi);

            pi = i*csides+csides+j;
            index.push_back(pi);
            normalindex.push_back(pi);

            pi = j < csides-1 ? i*csides+1+j : i*csides;
            index.push_back(pi);
            normalindex.push_back(pi);

            pi = i*csides+csides+j;
            index.push_back(pi);
            normalindex.push_back(pi);

            pi = j < csides-1 ? i*csides+csides+1+j : i*csides+csides;
            index.push_back(pi);
            normalindex.push_back(pi);

            pi = j < csides-1 ? i*csides+1+j : i*csides;
            index.push_back(pi);
            normalindex.push_back(pi);
        }

    // Caps
    // - Caps normals
    const unsigned long ci = static_cast<unsigned long>( vectors.size() );
    vectors.push_back(Vector3F(0,-1,0));

    float c = cos(cTorus.angle());
    float s = sin(cTorus.angle());
    vectors.push_back(Vector3F(-s,c,0));

    // - Caps centers
    points.push_back(Vector3F(center, 0, 0));
    points.push_back(Vector3F(c*center, s*center, 0));

    // - Caps indexes
    for (unsigned long j = 0; j < csides; j++)
    {
        index.push_back(j);
        index.push_back(j < csides-1 ? 1+j : 0);
        index.push_back(ci);
        normalindex.push_back(ci);
        normalindex.push_back(ci);
        normalindex.push_back(ci);
    }

    for (unsigned long j = 0; j < csides; j++)
    {
        index.push_back(tsides*csides+j);
        index.push_back( j < csides-1 ? tsides*csides+1+j : tsides*csides);
        index.push_back(ci+1);
        normalindex.push_back(ci+1);
        normalindex.push_back(ci+1);
        normalindex.push_back(ci+1);
    }

    Mesh result;
    result.positions = points;
    result.positionIndex = index;
    result.normals = vectors;
    result.normalIndex = normalindex;
    return result;
}

static const float pyramid[] = {
    .5, .5, -.5,
    .5, -.5, -.5,
    -.5, -.5, -.5,
    -.5, .5, -.5,
    .5, .5, .5,
    .5, -.5, .5,
    -.5, -.5, .5,
    -.5, .5, .5,
};

const Mesh RVMMeshHelper2::makePyramid(const Primitives::Pyramid& inP, const float& maxSideSize, const int& minSides)
{
    // Coordinates
    vector<Vector3F> points;
    for (int i = 0; i < 8; i++)
    {
        Vector3F p;
        p[0] = i < 4 ? pyramid[i * 3] * inP.xbottom() - inP.xoffset() * 0.5f: pyramid[i * 3] * inP.xtop() + inP.xoffset() * 0.5f;
        p[1] = i < 4 ? pyramid[i * 3 + 1] * inP.ybottom() - inP.yoffset() * 0.5f  : pyramid[i * 3 + 1] * inP.ytop() + inP.yoffset() * 0.5f;
        p[2] = pyramid[i * 3 + 2] * inP.height();
        points.push_back(p);
    }
    vector<unsigned long> index;
    // Sides
    for (int i = 0; i < 4; i++) {
        if (!points[i].equals(points[i == 3 ? 0 : i + 1]) && !points[i == 3 ? 0 : i + 1].equals(points[i+4]) && !points[i].equals(points[i+4])) {
            index.push_back(i);
            index.push_back(i + 4);
            index.push_back(i == 3 ? 0 : i + 1);
        }
        if (!points[i == 3 ? 0 : i+1].equals(points[i == 3 ? 4 : i+5]) && !points[i == 3 ? 4 : i+5].equals(points[i+4]) && !points[i == 3 ? 0 : i+1].equals(points[i+4])) {
            index.push_back(i == 3 ? 0 : i+1);
            index.push_back(i+4);
            index.push_back(i == 3 ? 4 : i+5);
        }
    }
    // Caps
    if (!points[0].equals(points[1]) && !points[1].equals(points[2]) && !points[0].equals(points[2])) {
        index.push_back(0);
        index.push_back(1);
        index.push_back(2);
        index.push_back(0);
        index.push_back(2);
        index.push_back(3);
    }
    if (!points[4].equals(points[5]) && !points[5].equals(points[6]) && !points[4].equals(points[6])) {
        index.push_back(4);
        index.push_back(6);
        index.push_back(5);
        index.push_back(4);
        index.push_back(7);
        index.push_back(6);
    }
    Mesh result;
    result.positions = points;
    result.positionIndex = index;
    return result;
}

unsigned long RVMMeshHelper2::infoCylinderNumSides(const Primitives::Cylinder &cylinder, float maxSideSize, unsigned long minSides)
{
    return std::max(minSides, static_cast<unsigned long>(2 * M_PI * cylinder.radius() / maxSideSize));
}

const Mesh RVMMeshHelper2::makeCylinder(const Primitives::Cylinder &cylinder, unsigned long sides)
{
    const float halfHeight = cylinder.height() / 2;

    vector<Vector3F> positions;
    vector<Vector3F> normals;
    const float d = float(2.0f * M_PI / static_cast<float>(sides));

    vector<unsigned long> positionIndex;
    vector<unsigned long> normalIndex;

    const unsigned long nrTrianglesSide = 2 * sides;

    for (unsigned long i = 0; i < sides; i++)
    {
        // Dimensions in x and y, z is height
        const float x = sin(d*static_cast<float>(i)); // [0..1]
        const float y = -cos(d*static_cast<float>(i)); // [-1..0]

        positions.push_back(Vector3F(x*cylinder.radius(), y*cylinder.radius(), -halfHeight));
        positions.push_back(Vector3F(x*cylinder.radius(), y*cylinder.radius(), +halfHeight));
        normals.push_back(Vector3F(x,y,0));

        const unsigned long v0 = i * 2;
        const unsigned long v1 = v0 + 1;
        const unsigned long v2 = (v0 + 2) % nrTrianglesSide;
        const unsigned long v3 = (v0 + 3) % nrTrianglesSide;

        const unsigned long n0 = i;
        const unsigned long n1 = (n0 + 1) % sides;

        // First triangle (CW: 0, 2, 1)
        positionIndex.push_back(v0);
        normalIndex.push_back(n0);
        positionIndex.push_back(v2);
        normalIndex.push_back(n1);
        positionIndex.push_back(v1);
        normalIndex.push_back(n0);

        // Second triangle (CW: 1, 2, 3)
        positionIndex.push_back(v1);
        normalIndex.push_back(n0);
        positionIndex.push_back(v2);
        normalIndex.push_back(n1);
        positionIndex.push_back(v3);
        normalIndex.push_back(n1);
    }

    Mesh result;
    result.positions = positions;
    result.normals = normals;
    result.positionIndex = positionIndex;
    result.normalIndex = normalIndex;
    return result;
}

unsigned long RVMMeshHelper2::infoSnoutNumSides(const Primitives::Snout &snout, float maxSideSize, unsigned long minSides)
{
    return std::max(minSides, static_cast<unsigned long>(2.0f * M_PI * std::max(snout.dbottom(), snout.dtop()) / maxSideSize));
}

const Mesh RVMMeshHelper2::makeSnout(const Primitives::Snout& snout, unsigned long sides)
{
    const float rbottom = snout.dbottom();
    const float rtop = snout.dtop();
    const float height = snout.height();
    const float xoffset = snout.xoffset();
    const float yoffset = snout.yoffset();

    vector<unsigned long> index;
    vector<Vector3F> points;
    vector<unsigned long> normalindex;
    vector<Vector3F> vectors;

    const float hh = height / 2;

    // Vector3Fes and normals
    Vector3F v;
    Vector3F n;
    const float da = 2.0f * M_PI / static_cast<float>(sides);
    for (unsigned long i = 0; i < sides; i++)
    {
        const float a = static_cast<float>(i)* da;
        const float c = cos(a);
        const float s = sin(a);

        v[0] = rbottom * c; v[1] = rbottom * s; v[2] = -hh;
        points.push_back(v);
        v[0] = rtop * c + xoffset; v[1] = rtop * s + yoffset; v[2] = hh;
        points.push_back(v);
        if (height > 0.0f)
        {
            float dh = sqrt(((rtop * c + xoffset - rbottom * c)*(rtop * c + xoffset - rbottom * c) + (rtop * s + yoffset - rbottom * s)*(rtop * s + yoffset - rbottom * s)) / (height*height));
            n[0] = c; n[1] = s; n[2] = (rtop < rbottom) ? dh : -dh;
        }
        else {
            n[0] = 0; n[1] = 0; n[2] = 1;
        }
        n.normalize();
        vectors.push_back(n);
    }

    // Sides
    for (unsigned long i = 0; i < sides; i++)
    {
        index.push_back(i * 2);
        index.push_back(i < sides - 1 ? i * 2 + 2 : 0);
        index.push_back(i * 2 + 1);

        normalindex.push_back(i);
        normalindex.push_back(i < sides - 1 ? i + 1 : 0);
        normalindex.push_back(i);

        index.push_back(i < sides - 1 ? i * 2 + 2 : 0);
        index.push_back(i < sides - 1 ? i * 2 + 3 : 1);
        index.push_back(i * 2 + 1);

        normalindex.push_back(i < sides - 1 ? i + 1 : 0);
        normalindex.push_back(i < sides - 1 ? i + 1 : 0);
        normalindex.push_back(i);
    }

    // Caps
    // - Caps normals
    const unsigned long nci = static_cast<unsigned long>(vectors.size());
    n[0] = 0; n[1] = 0; n[2] = -1;
    vectors.push_back(n);
    n[0] = 0; n[1] = 0; n[2] = 1;
    vectors.push_back(n);
    // - Caps centers
    const unsigned long ci = static_cast<unsigned long>(points.size());
    v[0] = 0; v[1] = 0; v[2] = -hh;
    points.push_back(v);
    v[0] = xoffset; v[1] = yoffset; v[2] = hh;
    points.push_back(v);
    // - Caps indexes
    for (unsigned long j = 0; j < sides; j++)
    {
        index.push_back(j * 2);
        index.push_back(ci);
        index.push_back(j < sides - 1 ? (j + 1) * 2 : 0);
        normalindex.push_back(nci);
        normalindex.push_back(nci);
        normalindex.push_back(nci);
    }

    for (unsigned long j = 0; j < sides; j++)
    {
        index.push_back(j * 2 + 1);
        index.push_back(j < sides - 1 ? j * 2 + 3 : 1);
        index.push_back(ci + 1);
        normalindex.push_back(nci + 1);
        normalindex.push_back(nci + 1);
        normalindex.push_back(nci + 1);
    }

    Mesh result;
    result.positions = points;
    result.positionIndex = index;
    result.normals = vectors;
    result.normalIndex = normalindex;
    return result;
}

std::pair<unsigned long, unsigned long> RVMMeshHelper2::infoEllipticalDishNumSides(const Primitives::EllipticalDish& eDish, float maxSideSize, unsigned long minSides)
{
    const float dishradius = eDish.diameter();
    const float secondradius = eDish.radius();

    unsigned long sides = std::max(minSides / 2, static_cast<unsigned long>(2.0f * M_PI * secondradius / maxSideSize) );
    unsigned long csides = std::max(minSides, static_cast<unsigned long>(2.0f * M_PI * dishradius / maxSideSize));

    return std::make_pair(sides, csides);
}

const Mesh RVMMeshHelper2::makeEllipticalDish(const Primitives::EllipticalDish& eDish, unsigned long sides, unsigned long csides)
{
    vector<unsigned long> index;
    vector<Vector3F> points;
    vector<Vector3F> vectors;

    const float dishradius = eDish.diameter();
    const float secondradius = eDish.radius();

    // Vector3Fes and normals
    Vector3F v;
    Vector3F n;

    const float da = M_PI / 2.0f / static_cast<float>( sides );
    const float da2 = 2.0f * M_PI / static_cast<float>( csides );
    for (unsigned long i = 0; i < sides; i++)
    {
        const float a = static_cast<float>(i) * da;
        const float c = cos(a);
        const float s = sin(a);

        for (unsigned long j = 0; j < csides; j++)
        {
            const float a2 = static_cast<float>(j) * da2;
            const float C = cos(a2);
            const float S = sin(a2);

            v[0] = dishradius * C * c; v[1] = dishradius * S * c; v[2] = secondradius * s;
            points.push_back(v);
            n[0] = secondradius * C * c; n[1] = secondradius * S * c; n[2] = dishradius * s;
            n.normalize();
            vectors.push_back(n);
        }
    }

    v[0] = 0; v[1] = 0; v[2] = secondradius;
    points.push_back(v);
    n[0] = 0; n[1] = 0; n[2] = 1;
    vectors.push_back(n);

    // Sides
    for (unsigned long i = 0; i < sides - 1; i++)
        for (unsigned long j = 0; j < csides; j++)
        {
            index.push_back(i*csides+j);
            index.push_back(j < csides-1 ? i*csides+1+j : i*csides);
            index.push_back(i*csides+csides+j);

            index.push_back(i*csides+csides+j);
            index.push_back(j < csides-1 ? i*csides+1+j : i*csides);
            index.push_back(j < csides-1 ? i*csides+csides+1+j : i*csides+csides);
        }

    for (unsigned long i = 0; i < csides; i++)
    {
        index.push_back(csides*(sides-1) + i);
        index.push_back(i == csides-1 ? csides*(sides-1) : csides*(sides-1) + i+1);
        index.push_back(static_cast<unsigned long >(points.size()) - 1);
    }

    Mesh result;
    result.positions = points;
    result.positionIndex = index;
    result.normals = vectors;

    return result;
}

const Mesh RVMMeshHelper2::makeSphericalDish(const Primitives::SphericalDish& sDish, const float& maxSideSize, const int& minSides)
{
    const float dishradius = sDish.diameter() / 2.0f;

    // Asking for a sphere...
    if (sDish.height() >= dishradius * 2)
    {
        Primitives::Sphere s;
        s.diamater = dishradius * 2;

        return makeSphere(s, maxSideSize, minSides);
    }

    vector<unsigned long> index;
    vector<Vector3F> points;
    vector<Vector3F> vectors;

    float radius = (dishradius*dishradius + sDish.height()*sDish.height()) / (2 * sDish.height());
    float angle = asin(1 - sDish.height() / radius);
    int csides = int(2*M_PI * radius / maxSideSize);
    if (csides < minSides) {
        csides = minSides;
    }
    int sides = csides;

    // Position and normals
    Vector3F v;
    Vector3F n;
    for (int i = 0; i < sides; i++) {
        float c = (float)cos(angle + (M_PI/2 - angle) / sides * i);
        float s = (float)sin(angle + (M_PI/2 - angle) / sides * i);
        for (int j = 0; j < csides; j++) {
            float C = (float)cos(2*M_PI / csides * j);
            float S = (float)sin(2*M_PI / csides * j);
            v[0] = radius * C * c; v[1] = radius * S * c; v[2] = -(radius - sDish.height() - radius * s);
            points.push_back(v);
            n[0] = radius * C * c; n[1] = radius * S * c; n[2] = radius * s;
            n.normalize();
            vectors.push_back(n);
        }
    }
    v[0] = 0; v[1] = 0; v[2] = sDish.height();
    points.push_back(v);
    n[0] = 0; n[1] = 0; n[2] = 1;
    vectors.push_back(n);

    // Sides
    for (int i = 0; i < sides-1; i++) {
        for (int j = 0; j < csides; j++) {

            unsigned long pi = i*csides+j;
            index.push_back(pi);
            pi = j < csides-1 ? i*csides+1+j : i*csides;
            index.push_back(pi);
            pi = i*csides+csides+j;
            index.push_back(pi);

            pi = i*csides+csides+j;
            index.push_back(pi);
            pi = j < csides-1 ? i*csides+1+j : i*csides;
            index.push_back(pi);
            pi = j < csides-1 ? i*csides+csides+1+j : i*csides+csides;
            index.push_back(pi);
        }
    }
    for (int i = 0; i < csides; i++)
    {
        unsigned long pi = csides*(sides-1) + i;
        index.push_back(pi);
        pi = i == csides-1 ? csides*(sides-1) : csides*(sides-1) + i+1;
        index.push_back(pi);
        pi = static_cast<unsigned long>(points.size()) - 1;
        index.push_back(pi);
    }

    Mesh result;
    result.positions = points;
    result.positionIndex = index;
    result.normals = vectors;
    return result;
}

pair<int, bool> createIndex( std::vector<Vertex>& references, const Vertex &newValue )
{
    unsigned int results = std::find( references.begin(), references.end(), newValue )
                                    - references.begin();
    if ( results == references.size() ) {
        references.push_back( newValue );
        return make_pair(results, true);
    }
    return make_pair(results, false);
}

void RVMMeshHelper2::tesselateFacetGroup(const std::vector<std::vector<std::vector<Vertex> > >& vertices, Mesh* userData) {
    GLUtesselator *tobj = gluNewTess();
    vector<Vertex> indexedVertices;
    vector<unsigned long> indexArray;

    gluTessCallback(tobj, GLU_TESS_EDGE_FLAG_DATA, (void (CALLBACK *)()) tessEdgeFlag);
    gluTessCallback(tobj, GLU_TESS_VERTEX_DATA,    (void (CALLBACK *)()) tessVertexData);
    gluTessCallback(tobj, GLU_TESS_COMBINE_DATA,   (void (CALLBACK *)()) tessCombineData);
    gluTessCallback(tobj, GLU_ERROR,               (void (CALLBACK *)()) tessError);

    unsigned long np = 0;

    for (unsigned int i = 0; i < vertices.size(); i++) {
        for (unsigned int j = 0; j < vertices[i].size(); j++) {
            for (unsigned int k = 0; k < vertices[i][j].size(); k++) {
                pair<int, bool> res = createIndex(indexedVertices, vertices[i][j][k]);
                indexArray.push_back(res.first);
                if(res.second) {
                    userData->positions.push_back(vertices[i][j][k].first);
                    userData->normals.push_back(vertices[i][j][k].second);
                }
                np++;
            }
        }
    }

    unsigned long tessIndex = 0;
    for (unsigned int i = 0; i < vertices.size(); i++) {
        gluTessBeginPolygon(tobj, userData);
        for (unsigned int j = 0; j < vertices[i].size(); j++) {
            gluTessBeginContour(tobj);

            double coords[3];
            for (unsigned int k = 0; k < vertices[i][j].size(); k++) {
                Vector3F vertex(vertices[i][j].at(k).first);
                coords[0] = vertex[0];
                coords[1] = vertex[1];
                coords[2] = vertex[2];
                gluTessVertex(tobj, coords, (void*)indexArray[tessIndex]);
                tessIndex++;
            }
            gluTessEndContour(tobj);
        }
        gluTessEndPolygon(tobj);
    }
    gluDeleteTess(tobj);
}
