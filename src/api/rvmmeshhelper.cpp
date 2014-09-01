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

#include <iostream>
#include <algorithm>

#ifdef _WIN32
  #include "windows.h"
#endif

#include <GL/gl.h>
#include <GL/glu.h>

#ifndef WIN32
#  define CALLBACK
#endif /* !WIN32 */

#ifdef _MSC_VER
#define _USE_MATH_DEFINES // For PI under VC++
#endif

#include <math.h>

using namespace std;

RVMMeshHelper2::RVMMeshHelper2()
{
}

static const float cube[] = {
    -.5, -.5, -.5,
    .5, -.5, -.5,
    .5, .5, -.5,
    -.5, .5, -.5,
    -.5, -.5, .5,
    .5, -.5, .5,
    .5, .5, .5,
    -.5, .5, .5,
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

void CALLBACK tessCombineData(GLdouble newVert[3], GLdouble *neighbourVert[4], GLfloat neighborWeight[4], void **outData, void * polygon_data) {
    Mesh* userData = (Mesh*) polygon_data;
    unsigned long index = userData->positions.size();
    userData->positions.push_back(Vector3F(float(newVert[0]), float(newVert[1]), float(newVert[2])));
    userData->normals.push_back(Vector3F(0.0f, 1.0f, 0.0f));
    *outData = (void*)index;
};

const Mesh RVMMeshHelper2::makeBox(const float& x, const float &y, const float &z, const float &maxSideSize, const int &minSides) {
    vector<Vector3F> points;
    for (int i = 0; i < 8; i++) {
        Vector3F point;
        point[0] = cube[i*3] * x;
        point[1] = cube[i*3+1] * y;
        point[2] = cube[i*3+2] * z;
        points.push_back(point);
    }
    vector<unsigned long> index;
    for (int i = 0; i < 4; i++) {
        index.push_back(i);
        index.push_back(i == 3 ? 0 : i+1);
        index.push_back(i+4);
        index.push_back(i == 3 ? 0 : i+1);
        index.push_back(i == 3 ? 4 : i+5);
        index.push_back(i+4);
    }

	vector<int> findex(3, 0);
    index.push_back(0);
	index.push_back(2);
	index.push_back(1);
    index.push_back(0);
	index.push_back(3);
	index.push_back(2);
    index.push_back(4);
	index.push_back(5);
	index.push_back(6);
    index.push_back(4);
	index.push_back(6);
	index.push_back(7);

	Mesh result;
	result.positions = points;
	result.positionIndex = index;
	return result;
}

const Mesh RVMMeshHelper2::makeSphere(const float& radius, const float& maxSideSize, const int& minSides) {
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

const Mesh RVMMeshHelper2::makeRectangularTorus(const float& rinside,
                                                const float& routside,
                                                const float& height,
                                                const float& angle, const float& maxSideSize, const int& minSides) {
    vector<unsigned long> index;
    vector<Vector3F> points;

	vector<unsigned long> normalindex;
    vector<Vector3F> vectors;

    int sides = int(angle * routside / maxSideSize);
    if (sides < minSides) {
        sides = minSides;
    }

    // Vertexes and normals
    Vector3F v;
    vectors.push_back(Vector3F(0,0,-1));
	vectors.push_back(Vector3F(0,0,1));

	for (int i = 0; i < sides+1; i++) {
        float c = cos(angle / sides * i);
        float s = sin(angle / sides * i);
        v[0] = rinside * c; v[1] = rinside * s; v[2] = -height/2.f;
        points.push_back(v);
        v[0] = routside * c; v[1] = routside * s; v[2]= -height/2.f;
        points.push_back(v);
        v[0] = routside * c; v[1] = routside * s; v[2] = height/2.f;
        points.push_back(v);
        v[0] = rinside * c; v[1] = rinside * s; v[2] = height/2.f;
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
    int nci = vectors.size();
    vectors.push_back(Vector3F(0,-1,0));
    float c = cos(angle);
    float s = sin(angle);
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

const Mesh RVMMeshHelper2::makeCircularTorus(const float& rinside,
                                                                                             const float& routside,
                                                                                             const float& angle, const float& maxSideSize, const int& minSides) {
    vector<unsigned long> index;
    vector<Vector3F> points;
    vector<unsigned long> normalindex;
    vector<Vector3F> vectors;

    int tsides = int(angle * rinside / maxSideSize);
    if (tsides < minSides) {
        tsides = minSides;
    }
    int csides = int(2*M_PI * routside / maxSideSize);
    if (csides < minSides) {
        csides = minSides;
    }

    // Vertexes and normals
    float rcenter = routside;
    float center = rinside;
    Vector3F v;
    Vector3F n;
    for (int i = 0; i < tsides+1; i++) {
        float c = cos(angle / tsides * i);
        float s = sin(angle / tsides * i);
        for (int j = 0; j < csides; j++) {
            float C = (float)cos(2 * M_PI / csides * j);
            float S = (float)sin(2 * M_PI / csides * j);
            v[0] = (rcenter * C + center) * c;
            v[1] = (rcenter * C + center) * s;
            v[2] = rcenter * S;
            points.push_back(v);
            n[0] = C*c;
            n[1] = C*s;
            n[2] = S;
            vectors.push_back(n);
        }
    }

    // Sides
    for (int i = 0; i < tsides; i++) {
        for (int j = 0; j < csides; j++) {
            unsigned long pi = i*csides+j;
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
    }

    // Caps
    // - Caps normals
    int ci = vectors.size();
    vectors.push_back(Vector3F(0,-1,0));

	float c = cos(angle);
    float s = sin(angle);
    vectors.push_back(Vector3F(-s,c,0));

	// - Caps centers
    points.push_back(Vector3F(center, 0, 0));
	points.push_back(Vector3F(c*center, s*center, 0));

	// - Caps indexes
    for (int j = 0; j < csides; j++) {
        index.push_back(j);
        index.push_back(j < csides-1 ? 1+j : 0);
        index.push_back(ci);
        normalindex.push_back(ci);
		normalindex.push_back(ci);
		normalindex.push_back(ci);
    }
    for (int j = 0; j < csides; j++) {
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

const Mesh RVMMeshHelper2::makePyramid(const float& xbottom, const float& ybottom, const float& xtop, const float& ytop, const float& xoffset, const float& yoffset, const float& height, const float& maxSideSize, const int& minSides) {
    // Coordinates
    vector<Vector3F> points;
    for (int i = 0; i < 8; i++) {
        Vector3F p;
        p[0] = i < 4 ? pyramid[i*3] * xbottom : pyramid[i*3] * xtop + xoffset;
        p[1] = i < 4 ? pyramid[i*3+1] * ybottom : pyramid[i*3+1] * ytop + yoffset;
        p[2] = pyramid[i*3+2] * height;
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

const Mesh RVMMeshHelper2::makeCylinder(const float& radius, const float& height, const float& maxSideSize, const int& minSides) {
    int s = int(2 * M_PI * radius / maxSideSize);
    if (s < minSides) {
        s = minSides;
    }
    float halfHeight = height / 2;

    vector<Vector3F> positions;
    vector<Vector3F> normals;
    float d = float(2*M_PI/(float)s);

	vector<unsigned long> positionIndex;
	vector<unsigned long> normalIndex;

	int nrTrianglesSide = 2*s;

	for (int i = 0; i < s; i++) {
        // Dimensions in x and y, z is height
		float x = sin(d*(float)i); // [0..1]
		float y = -cos(d*(float)i); // [-1..0]

		positions.push_back(Vector3F(x*radius, y*radius, -halfHeight));
		positions.push_back(Vector3F(x*radius, y*radius, +halfHeight));
		normals.push_back(Vector3F(x,y,0));

		unsigned long v0 = i * 2;
		unsigned long v1 = v0 + 1;
		unsigned long v2 = (v0 + 2) % nrTrianglesSide;
		unsigned long v3 = (v0 + 3) % nrTrianglesSide;

		unsigned long n0 = i;
		unsigned long n1 = (n0+1) % s;

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

const Mesh RVMMeshHelper2::makeSnout(const float& rbottom, const float& rtop, const float& height, const float& xoffset, const float& yoffset, const float& maxSideSize, const int& minSides) {
    vector<unsigned long> index;
    vector<Vector3F> points;
    vector<unsigned long> normalindex;
    vector<Vector3F> vectors;
    float hh = height / 2;

    int sides = int(2*M_PI * (rbottom > rtop ? rbottom : rtop) / maxSideSize);
    if (sides < minSides) {
        sides = minSides;
    }

    // Vector3Fes and normals
    Vector3F v;
    Vector3F n;
    for (int i = 0; i < sides; i++) {
        float c = (float)cos(2*M_PI / sides * i);
        float s = (float)sin(2*M_PI / sides * i);
        v[0] = rbottom * c; v[1] = rbottom * s; v[2] = -hh;
        points.push_back(v);
        v[0] = rtop * c + xoffset; v[1] = rtop * s + yoffset; v[2] = hh;
        points.push_back(v);
        if (height > 0) {
            float dh = sqrt(((rtop * c + xoffset - rbottom * c)*(rtop * c + xoffset - rbottom * c) + (rtop * s + yoffset - rbottom * s)*(rtop * s + yoffset - rbottom * s)) / (height*height));
            n[0] = c; n[1] = s; n[2] = (rtop < rbottom) ? dh : -dh;
            n.normalize();
        } else {
            n[0] = 0; n[1] = 0; n[2] = 1;
        }
        vectors.push_back(n);
    }

    // Sides
    for (int i = 0; i < sides; i++) {
        index.push_back(i*2);
		index.push_back(i < sides - 1 ? i*2+2 : 0);
		index.push_back(i*2+1);

		normalindex.push_back(i);
		normalindex.push_back(i < sides - 1 ? i+1 : 0);
		normalindex.push_back(i);

		index.push_back(i < sides - 1 ? i*2+2 : 0);
		index.push_back(i < sides - 1 ? i*2+3 : 1);
		index.push_back(i*2+1);

        normalindex.push_back(i < sides - 1 ? i+1 : 0);
		normalindex.push_back(i < sides - 1 ? i+1 : 0);
		normalindex.push_back(i);
    }

    // Caps
    // - Caps normals
    int nci = vectors.size();
    n[0] = 0; n[1] = 0; n[2] = -1;
    vectors.push_back(n);
    n[0] = 0; n[1] = 0; n[2] = 1;
    vectors.push_back(n);
    // - Caps centers
	int ci = points.size();
    v[0] = 0; v[1] = 0; v[2] = -hh;
    points.push_back(v);
    v[0] = xoffset; v[1] = yoffset; v[2] = hh;
    points.push_back(v);
    // - Caps indexes
    for (int j = 0; j < sides; j++) {
        index.push_back(j*2);
        index.push_back(ci);
        index.push_back(j < sides-1 ? (j+1)*2 : 0);
        normalindex.push_back(nci);
		normalindex.push_back(nci);
		normalindex.push_back(nci);
    }
    for (int j = 0; j < sides; j++) {
        index.push_back(j*2+1);
		index.push_back(j < sides-1 ? j*2 + 3 : 1);
		index.push_back(ci+1);
        normalindex.push_back(nci+1);
		normalindex.push_back(nci+1);
		normalindex.push_back(nci+1);
    }

	Mesh result;
	result.positions = points;
	result.positionIndex = index;
	result.normals = vectors;
	result.normalIndex = normalindex;
    return result;
}

const Mesh RVMMeshHelper2::makeEllipticalDish(const float& dishradius, const float& secondradius, const float& maxSideSize, const int& minSides) {
    vector<unsigned long> index;
    vector<Vector3F> points;
    vector<unsigned long> normalindex;
    vector<Vector3F> vectors;

    float hd = dishradius;
    int sides = int(2*M_PI * secondradius / maxSideSize);
    if (sides < minSides / 2) {
        sides = minSides / 2;
    }
    int csides = int(2*M_PI * hd / maxSideSize);
    if (csides < minSides) {
        csides = minSides;
    }

    // Vector3Fes and normals
    Vector3F v;
    Vector3F n;
    for (int i = 0; i < sides; i++) {
        float c = (float)cos(M_PI / 2 / sides * i);
        float s = (float)sin(M_PI / 2 / sides * i);
        for (int j = 0; j < csides; j++) {
            float C = (float)cos(2*M_PI / csides * j);
            float S = (float)sin(2*M_PI / csides * j);
            v[0] = hd * C * c; v[1] = hd * S * c; v[2] = secondradius * s;
            points.push_back(v);
            n[0] = secondradius * C * c; n[1] = secondradius * S * c; n[2] = hd * s;
            n.normalize();
            vectors.push_back(n);
        }
    }
    v[0] = 0; v[1] = 0; v[2] = secondradius;
    points.push_back(v);
    n[0] = 0; n[1] = 0; n[2] = 1;
    vectors.push_back(n);

    // Sides
    for (int i = 0; i < sides-1; i++) {
        for (int j = 0; j < csides; j++) {
            index.push_back(i*csides+j);
			index.push_back(j < csides-1 ? i*csides+1+j : i*csides);
			index.push_back(i*csides+csides+j);

            index.push_back(i*csides+csides+j);
			index.push_back(j < csides-1 ? i*csides+1+j : i*csides);
			index.push_back(j < csides-1 ? i*csides+csides+1+j : i*csides+csides);
        }
    }
    for (int i = 0; i < csides; i++) {
        index.push_back(csides*(sides-1) + i);
		index.push_back(i == csides-1 ? csides*(sides-1) : csides*(sides-1) + i+1);
		index.push_back(points.size()-1);
    }

	Mesh result;
	result.positions = points;
	result.positionIndex = index;
	result.normals = vectors;
	//result.normalIndex = normalindex;
    return result;
}

const Mesh RVMMeshHelper2::makeSphericalDish(const float& dishradius, const float& height, const float& maxSideSize, const int& minSides) {

    // Asking for a sphere...
    if (height >= dishradius * 2) {
        cout << "Sphere" << endl;
		return makeSphere(dishradius * 2, maxSideSize, minSides);
    }

    vector<unsigned long> index;
    vector<Vector3F> points;
    vector<Vector3F> vectors;

    float radius = (dishradius*dishradius + height*height) / (2*height);
    float angle = asin(1-height/radius);
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
            v[0] = radius * C * c; v[1] = radius * S * c; v[2] = -(radius - height -radius * s);
            points.push_back(v);
            n[0] = radius * C * c; n[1] = radius * S * c; n[2] = radius * s;
            n.normalize();
            vectors.push_back(n);
        }
    }
    v[0] = 0; v[1] = 0; v[2] = height;
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
    for (int i = 0; i < csides; i++) {
        unsigned long pi = csides*(sides-1) + i;
		index.push_back(pi);
        pi = i == csides-1 ? csides*(sides-1) : csides*(sides-1) + i+1;
		index.push_back(pi);
        pi = points.size()-1;
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
