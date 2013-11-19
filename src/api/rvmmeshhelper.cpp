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

#ifdef _MSC_VER
#define _USE_MATH_DEFINES // For PI under VC++
#endif

#include <math.h>

using namespace std;

RVMMeshHelper::RVMMeshHelper()
{
}

static vector<float> midpoint(const vector<float>& p1, const vector<float>& p2) {
    vector<float> mp;
    mp.push_back((p1[0] + p2[0]) / 2.f);
    mp.push_back((p1[1] + p2[1]) / 2.f);
    mp.push_back((p1[2] + p2[2]) / 2.f);
    return mp;
}

static vector<float> normalize(const vector<float>& v) {
    vector<float> n = v;
    double mag = v[0] * v[0] + v[1] * v[1] + v[2] * v[2];
    if (mag != 0) {
        mag = 1. / sqrt(mag);
        n[0] = float(v[0] * mag);
        n[1] = float(v[1] * mag);
        n[2] = float(v[2] * mag);
    }
    return n;
}

static vector<float> cross(const vector<float>& v1, const vector<float>& v2) {
    vector<float> c(3, 0);
    c[0] = v1[1]*v2[2] - v2[1]*v1[2];
    c[1] = -v1[0]*v2[2] + v2[0]*v1[2];
    c[2] = v1[0]*v2[1] - v2[1]*v1[1];
    return c;
}

static bool equals(const vector<float>& p1, const vector<float>& p2) {
    return p1[0] == p2[0] && p1[1] == p2[1] && p1[2] == p2[2];
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

const pair<vector<vector<float> >, vector<vector<int> > > RVMMeshHelper::makeBox(const float& x, const float &y, const float &z, const float &maxSideSize, const int &minSides) {
    vector<vector<float> > points;
    for (int i = 0; i < 8; i++) {
        vector<float> point;
        point.push_back(cube[i*3] * x);
        point.push_back(cube[i*3+1] * y);
        point.push_back(cube[i*3+2] * z);
        points.push_back(point);
    }
    vector<vector<int> > index;
    for (int i = 0; i < 4; i++) {
        vector<int> findex;
        findex.push_back(i);
        findex.push_back(i == 3 ? 0 : i+1);
        findex.push_back(i+4);
        index.push_back(findex);
        findex.clear();
        findex.push_back(i == 3 ? 0 : i+1);
        findex.push_back(i == 3 ? 4 : i+5);
        findex.push_back(i+4);
        index.push_back(findex);
    }
    vector<int> findex(3, 0);
    findex[0] = 0; findex[1] = 2; findex[2] = 1;
    index.push_back(findex);
    findex[0] = 0; findex[1] = 3; findex[2] = 2;
    index.push_back(findex);
    findex[0] = 4; findex[1] = 5; findex[2] = 6;
    index.push_back(findex);
    findex[0] = 4; findex[1] = 6; findex[2] = 7;
    index.push_back(findex);
    return pair<vector<vector<float> >, vector<vector<int> > >(points, index);
}

static const float sphere[] = {
    0, 0, 1,
    1, 0, 0,
    0, 1, 0,
    -1, 0, 0,
    0, -1, 0,
    0, 0, -1,
};

static const int sphereindex[] = {
    0, 1, 2,
    0, 2, 3,
    0, 3, 4,
    0, 4, 1,
    2, 1, 5,
    3, 2, 5,
    4, 3, 5,
    1, 4, 5,
};

const pair<
        pair<vector<vector<float> >, vector<vector<int> > >,
        pair<vector<vector<float> >, vector<vector<int> > > > RVMMeshHelper::makeSphere(const float& radius, const float& maxSideSize, const int& minSides) {
    // Init sphere
    int s = 4;
    vector<vector<float> > vectors;
    for (int i = 0; i < 6; i++) {
        vector<float> p;
        p.push_back(sphere[i*3]);
        p.push_back(sphere[i*3+1]);
        p.push_back(sphere[i*3+2]);
        vectors.push_back(p);
    }
    vector<vector<int> > index;
    for (int i = 0; i < 8; i++) {
        vector<int> findex;
        findex.push_back(sphereindex[i*3]);
        findex.push_back(sphereindex[i*3+1]);
        findex.push_back(sphereindex[i*3+2]);
        index.push_back(findex);
    }
    while (s < minSides || 2 * M_PI * radius / s > maxSideSize) {
        s *= 2;
        vector<vector<int> > newindex;
        for (unsigned int i = 0; i < index.size(); i++) {
            vector<int> t = index[i];
            int npi = vectors.size();
            vectors.push_back(normalize(midpoint(vectors[t[0]], vectors[t[1]])));
            vectors.push_back(normalize(midpoint(vectors[t[1]], vectors[t[2]])));
            vectors.push_back(normalize(midpoint(vectors[t[2]], vectors[t[0]])));
            vector<int> t1;
            t1.push_back(t[0]);
            t1.push_back(npi);
            t1.push_back(npi+2);
            newindex.push_back(t1);
            vector<int> t2;
            t2.push_back(npi);
            t2.push_back(npi+1);
            t2.push_back(npi+2);
            newindex.push_back(t2);
            vector<int> t3;
            t3.push_back(t[1]);
            t3.push_back(npi+1);
            t3.push_back(npi);
            newindex.push_back(t3);
            vector<int> t4;
            t4.push_back(t[2]);
            t4.push_back(npi+2);
            t4.push_back(npi+1);
            newindex.push_back(t4);
        }
        index.swap(newindex);
    }
    vector<vector<float> > points;
    for (unsigned int i = 0; i < vectors.size(); i++) {
        vector<float> p;
        for (int j = 0; j < 3; j++) {
            p.push_back(vectors[i][j] * radius);
        }
        points.push_back(p);
    }
    pair<vector<vector<float> >, vector<vector<int> > > vertexes(points, index);
    pair<vector<vector<float> >, vector<vector<int> > > normals(vectors, index);
    return pair<pair<vector<vector<float> >, vector<vector<int> > >, pair<vector<vector<float> >, vector<vector<int> > > >(vertexes, normals);
}

const pair<
        pair<vector<vector<float> >, vector<vector<int> > >,
        pair<vector<vector<float> >, vector<vector<int> > > > RVMMeshHelper::makeRectangularTorus(const float& rinside,
                                                                                             const float& routside,
                                                                                             const float& height,
                                                                                             const float& angle, const float& maxSideSize, const int& minSides) {
    vector<vector<int> > index;
    vector<vector<float> > points;
    vector<vector<int> > normalindex;
    vector<vector<float> > vectors;

    int sides = int(angle * routside / maxSideSize);
    if (sides < minSides) {
        sides = minSides;
    }

    // Vertexes and normals
    vector<float> v(3, 0);
    vector<float> n(3, 0);
    n[2] = -1;
    vectors.push_back(n);
    n[2] = 1;
    vectors.push_back(n);
    for (int i = 0; i < sides+1; i++) {
        float c = cos(angle / sides * i);
        float s = sin(angle / sides * i);
        v[0] = rinside * c; v[1] = rinside * s; v[2] = -height/2.f;
        points.push_back(v);
        v[0] = routside * c; v[1] = routside * s; v[2] = -height/2.f;
        points.push_back(v);
        v[0] = routside * c; v[1] = routside * s; v[2] = height/2.f;
        points.push_back(v);
        v[0] = rinside * c; v[1] = rinside * s; v[2] = height/2.f;
        points.push_back(v);
        n[0] = c; n[1] = s; n[2] = 0;
        vectors.push_back(n);
        n[0] = -c; n[1] = -s; n[2] = 0;
        vectors.push_back(n);
    }

    // Sides
    vector<int> pi(3, 0);
    vector<int> ni(3, 0);
    for (int i = 0; i < sides; i++) {
        for (int j = 0; j < 4; j++) {
            pi[0] = i*4+j; pi[1] = i*4+4+j; pi[2] = j < 3 ? i*4+1+j : i*4;
            index.push_back(pi);
            for (int k = 0; k < 3; k++) ni[k] = j == 0 ? 0 : j == 1 ? i*2+2 : j == 2 ? 1 : i*2+3;
            if (j == 1 || j == 3) {
                ni[1] += 2;
            }
            normalindex.push_back(ni);
            pi[0] = i*4+4+j; pi[1] = j < 3 ? i*4+5+j : i*4+4; pi[2] = j < 3 ? i*4+1+j : i*4;
            index.push_back(pi);
            if (j == 1 || j == 3) {
                ni[0] += 2;
            }
            normalindex.push_back(ni);
        }
    }

    // Caps
    // - Caps normals
    int nci = vectors.size();
    n[0] = 0; n[1] = -1; n[2] = 0;
    vectors.push_back(n);
    float c = cos(angle);
    float s = sin(angle);
    n[0] = -s; n[1] = c; n[2] = 0;
    vectors.push_back(n);
    // - Caps indexes
    pi[0] = 0; pi[1] = 1; pi[2] = 2;
    index.push_back(pi);
    ni[0] = nci; ni[1] = nci; ni[2] = nci;
    normalindex.push_back(ni);
    pi[0] = 0; pi[1] = 2; pi[2] = 3;
    index.push_back(pi);
    normalindex.push_back(ni);
    //
    pi[0] = sides*4; pi[1] = sides*4+2; pi[2] = sides*4+1;
    index.push_back(pi);
    ni[0] = nci+1; ni[1] = nci+1; ni[2] = nci+1;
    normalindex.push_back(ni);
    pi[0] = sides*4; pi[1] = sides*4+3; pi[2] = sides*4+2;
    index.push_back(pi);
    normalindex.push_back(ni);

    pair<vector<vector<float> >, vector<vector<int> > > vertexes(points, index);
    pair<vector<vector<float> >, vector<vector<int> > > normals(vectors, normalindex);
    return pair<pair<vector<vector<float> >, vector<vector<int> > >, pair<vector<vector<float> >, vector<vector<int> > > >(vertexes, normals);
}

const pair<
        pair<vector<vector<float> >, vector<vector<int> > >,
        pair<vector<vector<float> >, vector<vector<int> > > > RVMMeshHelper::makeCircularTorus(const float& rinside,
                                                                                             const float& routside,
                                                                                             const float& angle, const float& maxSideSize, const int& minSides) {
    vector<vector<int> > index;
    vector<vector<float> > points;
    vector<vector<int> > normalindex;
    vector<vector<float> > vectors;

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
    vector<float> v(3, 0);
    vector<float> n(3, 0);
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
    vector<int> pi(3, 0);
    vector<int> ni(3, 0);
    for (int i = 0; i < tsides; i++) {
        for (int j = 0; j < csides; j++) {
            pi[0] = i*csides+j; pi[1] = i*csides+csides+j; pi[2] = j < csides-1 ? i*csides+1+j : i*csides;
            index.push_back(pi);
            normalindex.push_back(pi);
            pi[0] = i*csides+csides+j; pi[1] = j < csides-1 ? i*csides+csides+1+j : i*csides+csides; pi[2] = j < csides-1 ? i*csides+1+j : i*csides;
            index.push_back(pi);
            normalindex.push_back(pi);
        }
    }

    // Caps
    // - Caps normals
    int ci = vectors.size();
    n[0] = 0; n[1] = -1; n[2] = 0;
    vectors.push_back(n);
    float c = cos(angle);
    float s = sin(angle);
    n[0] = -s; n[1] = c; n[2] = 0;
    vectors.push_back(n);
    // - Caps centers
    v[0] = center; v[1] = 0; v[2] = 0;
    points.push_back(v);
    v[0] = c * center; v[1] = s * center; v[2] = 0;
    points.push_back(v);
    // - Caps indexes
    ni[0] = ci; ni[1] = ci; ni[2] = ci;
    for (int j = 0; j < csides; j++) {
        pi[0] = j; pi[1] = j < csides-1 ? 1+j : 0; pi[2] = ci;
        index.push_back(pi);
        normalindex.push_back(ni);
    }
    ni[0] = ci+1; ni[1] = ci+1; ni[2] = ci+1;
    for (int j = 0; j < csides; j++) {
        pi[0] = tsides*csides+j; pi[2] = j < csides-1 ? tsides*csides+1+j : tsides*csides; pi[1] = ci+1;
        index.push_back(pi);
        normalindex.push_back(ni);
    }

    pair<vector<vector<float> >, vector<vector<int> > > vertexes(points, index);
    pair<vector<vector<float> >, vector<vector<int> > > normals(vectors, normalindex);
    return pair<pair<vector<vector<float> >, vector<vector<int> > >, pair<vector<vector<float> >, vector<vector<int> > > >(vertexes, normals);
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

const pair<vector<vector<float> >, vector<vector<int> > > RVMMeshHelper::makePyramid(const float& xbottom, const float& ybottom, const float& xtop, const float& ytop, const float& xoffset, const float& yoffset, const float& height, const float& maxSideSize, const int& minSides) {
    // Coordinates
    vector<vector<float> > points;
    for (int i = 0; i < 8; i++) {
        vector<float> p;
        p.push_back(i < 4 ? pyramid[i*3] * xbottom : pyramid[i*3] * xtop + xoffset);
        p.push_back(i < 4 ? pyramid[i*3+1] * ybottom : pyramid[i*3+1] * ytop + yoffset);
        p.push_back(pyramid[i*3+2] * height);
        points.push_back(p);
    }
    vector<vector<int> > index;
    // Sides
    for (int i = 0; i < 4; i++) {
        if (!equals(points[i], points[i == 3 ? 0 : i + 1]) && !equals(points[i == 3 ? 0 : i + 1], points[i+4]) && !equals(points[i], points[i+4])) {
            vector<int> findex;
            findex.push_back(i);
            findex.push_back(i + 4);
            findex.push_back(i == 3 ? 0 : i + 1);
            index.push_back(findex);
        }
        if (!equals(points[i == 3 ? 0 : i+1], points[i == 3 ? 4 : i+5]) && !equals(points[i == 3 ? 4 : i+5], points[i+4]) && !equals(points[i == 3 ? 0 : i+1], points[i+4])) {
            vector<int> findex;
            findex.push_back(i == 3 ? 0 : i+1);
            findex.push_back(i+4);
            findex.push_back(i == 3 ? 4 : i+5);
            index.push_back(findex);
        }
    }
    // Caps
    if (!equals(points[0], points[1]) && !equals(points[1], points[2]) && !equals(points[0], points[2])) {
        vector<int> findex;
        findex.push_back(0);
        findex.push_back(1);
        findex.push_back(2);
        index.push_back(findex);
        findex.clear();
        findex.push_back(0);
        findex.push_back(2);
        findex.push_back(3);
        index.push_back(findex);
    }
    if (!equals(points[4], points[5]) && !equals(points[5], points[6]) && !equals(points[4], points[6])) {
        vector<int> findex;
        findex.push_back(4);
        findex.push_back(6);
        findex.push_back(5);
        index.push_back(findex);
        findex.clear();
        findex.push_back(4);
        findex.push_back(7);
        findex.push_back(6);
        index.push_back(findex);
    }
    return pair<vector<vector<float> >, vector<vector<int> > >(points, index);
}

const pair<
        pair<vector<vector<float> >, vector<vector<int> > >,
        pair<vector<vector<float> >, vector<vector<int> > > > RVMMeshHelper::makeCylinder(const float& radius, const float& height, const float& maxSideSize, const int& minSides) {
    int s = int(2 * M_PI * radius / maxSideSize);
    if (s < minSides) {
        s = minSides;
    }
    float hh = height / 2;

    vector<vector<float> > points;
    vector<vector<float> > vectors;
    float r = radius;
    for (int i = 0; i < s; i++) {
        vector<float> point;
        point.push_back((float)cos(2*M_PI*i/s));
        point.push_back((float)sin(2*M_PI*i/s));
        point.push_back(0);
        vectors.push_back(point);
        vectors.push_back(point);
        point[0] *= r;
        point[1] *= r;
        point[2] = -hh;
        points.push_back(point);
        point[2] = hh;
        points.push_back(point);
    }
    vector<vector<int> > index;
    for (int i = 0; i < s; i++) {
        vector<int> findex;
        findex.push_back(i*2);
        findex.push_back(i == s-1 ? 0 : i*2+2);
        findex.push_back(i*2+1);
        index.push_back(findex);
        findex.clear();
        findex.push_back(i == s-1 ? 0 : i*2+2);
        findex.push_back(i == s-1 ? 1 : i*2+3);
        findex.push_back(i*2+1);
        index.push_back(findex);
    }
    vector<vector<int> > normalindex = index;
    vector<float> point(3, 0);
    int c = points.size();
    point[2] = -hh;
    points.push_back(point);
    point[2] = hh;
    points.push_back(point);
    point[2] = -1;
    vectors.push_back(point);
    point[2] = 1;
    vectors.push_back(point);
    for (int i = 0; i < s; i++) {
        vector<int> findex;
        findex.push_back(i*2);
        findex.push_back(c);
        findex.push_back(i == s-1 ? 0 : i*2+2);
        index.push_back(findex);
        findex.clear();
        findex.push_back(i*2+1);
        findex.push_back(i == s-1 ? 1 : i*2+3);
        findex.push_back(c+1);
        index.push_back(findex);
        normalindex.push_back(vector<int>(3, c));
        normalindex.push_back(vector<int>(3, c+1));
    }

    pair<vector<vector<float> >, vector<vector<int> > > vertexes(points, index);
    pair<vector<vector<float> >, vector<vector<int> > > normals(vectors, normalindex);
    return pair<pair<vector<vector<float> >, vector<vector<int> > >, pair<vector<vector<float> >, vector<vector<int> > > >(vertexes, normals);
}

const pair<
        pair<vector<vector<float> >, vector<vector<int> > >,
        pair<vector<vector<float> >, vector<vector<int> > > > RVMMeshHelper::makeSnout(const float& rbottom, const float& rtop, const float& height, const float& xoffset, const float& yoffset, const float& maxSideSize, const int& minSides) {
    vector<vector<int> > index;
    vector<vector<float> > points;
    vector<vector<int> > normalindex;
    vector<vector<float> > vectors;
    float hh = height / 2;

    int sides = int(2*M_PI * (rbottom > rtop ? rbottom : rtop) / maxSideSize);
    if (sides < minSides) {
        sides = minSides;
    }

    // Vertexes and normals
    vector<float> v(3, 0);
    vector<float> n(3, 0);
    for (int i = 0; i < sides; i++) {
        float c = (float)cos(2*M_PI / sides * i);
        float s = (float)sin(2*M_PI / sides * i);
        v[0] = rbottom * c; v[1] = rbottom * s; v[2] = -hh;
        points.push_back(v);
        v[0] = rtop * c + xoffset; v[1] = rtop * s + yoffset; v[2] = hh;
        points.push_back(v);
        float dh = sqrt(((rtop * c + xoffset - rbottom * c)*(rtop * c + xoffset - rbottom * c) + (rtop * s + yoffset - rbottom * s)*(rtop * s + yoffset - rbottom * s)) / (height*height));
        n[0] = c; n[1] = s; n[2] = (rtop < rbottom) ? dh : -dh;
        n = normalize(n);
        vectors.push_back(n);
    }

    // Sides
    vector<int> pi(3, 0);
    vector<int> ni(3, 0);
    for (int i = 0; i < sides; i++) {
        pi[0] = i*2; pi[1] = i < sides - 1 ? i*2+2 : 0; pi[2] = i*2+1;
        index.push_back(pi);
        ni[0] = i; ni[1] = i < sides - 1 ? i+1 : 0; ni[2] = i;
        normalindex.push_back(ni);
        pi[0] = i < sides - 1 ? i*2+2 : 0; pi[1] = i < sides - 1 ? i*2+3 : 1; pi[2] = i*2+1;
        index.push_back(pi);
        ni[0] = i < sides - 1 ? i+1 : 0; ni[1] = i < sides - 1 ? i+1 : 0; ni[2] = i;
        normalindex.push_back(ni);
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
    ni[0] = nci; ni[1] = nci; ni[2] = nci;
    for (int j = 0; j < sides; j++) {
        pi[0] = j*2; pi[1] = ci; pi[2] = j < sides-1 ? (j+1)*2 : 0;
        index.push_back(pi);
        normalindex.push_back(ni);
    }
    ni[0] = nci+1; ni[1] = nci+1; ni[2] = nci+1;
    for (int j = 0; j < sides; j++) {
        pi[0] = j*2+1; pi[2] = ci+1; pi[1] = j < sides-1 ? j*2 + 3 : 1;
        index.push_back(pi);
        normalindex.push_back(ni);
    }

    pair<vector<vector<float> >, vector<vector<int> > > vertexes(points, index);
    pair<vector<vector<float> >, vector<vector<int> > > normals(vectors, normalindex);
    return pair<pair<vector<vector<float> >, vector<vector<int> > >, pair<vector<vector<float> >, vector<vector<int> > > >(vertexes, normals);
}

const pair<
        pair<vector<vector<float> >, vector<vector<int> > >,
        pair<vector<vector<float> >, vector<vector<int> > > > RVMMeshHelper::makeEllipticalDish(const float& dishradius, const float& secondradius, const float& maxSideSize, const int& minSides) {
    vector<vector<int> > index;
    vector<vector<float> > points;
    vector<vector<int> > normalindex;
    vector<vector<float> > vectors;

    float hd = dishradius;
    int sides = int(2*M_PI * secondradius / maxSideSize);
    if (sides < minSides / 2) {
        sides = minSides / 2;
    }
    int csides = int(2*M_PI * hd / maxSideSize);
    if (csides < minSides) {
        csides = minSides;
    }

    // Vertexes and normals
    vector<float> v(3, 0);
    vector<float> n(3, 0);
    for (int i = 0; i < sides; i++) {
        float c = (float)cos(M_PI / 2 / sides * i);
        float s = (float)sin(M_PI / 2 / sides * i);
        for (int j = 0; j < csides; j++) {
            float C = (float)cos(2*M_PI / csides * j);
            float S = (float)sin(2*M_PI / csides * j);
            v[0] = hd * C * c; v[1] = hd * S * c; v[2] = secondradius * s;
            points.push_back(v);
            n[0] = secondradius * C * c; n[1] = secondradius * S * c; n[2] = hd * s;
            vectors.push_back(normalize(n));
        }
    }
    v[0] = 0; v[1] = 0; v[2] = secondradius;
    points.push_back(v);
    n[0] = 0; n[1] = 0; n[2] = 1;
    vectors.push_back(n);

    // Sides
    vector<int> pi(3, 0);
    vector<int> ni(3, 0);
    for (int i = 0; i < sides-1; i++) {
        for (int j = 0; j < csides; j++) {
            pi[0] = i*csides+j; pi[2] = i*csides+csides+j; pi[1] = j < csides-1 ? i*csides+1+j : i*csides;
            index.push_back(pi);
            normalindex.push_back(pi);
            pi[0] = i*csides+csides+j; pi[2] = j < csides-1 ? i*csides+csides+1+j : i*csides+csides; pi[1] = j < csides-1 ? i*csides+1+j : i*csides;
            index.push_back(pi);
            normalindex.push_back(pi);
        }
    }
    for (int i = 0; i < csides; i++) {
        pi[0] = csides*(sides-1) + i; pi[1] = i == csides-1 ? csides*(sides-1) : csides*(sides-1) + i+1; pi[2] = points.size()-1;
        index.push_back(pi);
        normalindex.push_back(pi);
    }

    pair<vector<vector<float> >, vector<vector<int> > > vertexes(points, index);
    pair<vector<vector<float> >, vector<vector<int> > > normals(vectors, normalindex);
    return pair<pair<vector<vector<float> >, vector<vector<int> > >, pair<vector<vector<float> >, vector<vector<int> > > >(vertexes, normals);
}

const pair<
        pair<vector<vector<float> >, vector<vector<int> > >,
        pair<vector<vector<float> >, vector<vector<int> > > > RVMMeshHelper::makeSphericalDish(const float& dishradius, const float& height, const float& maxSideSize, const int& minSides) {

    // Asking for a sphere...
    if (height >= dishradius * 2) {
        return makeSphere(dishradius, maxSideSize, minSides);
    }

    vector<vector<int> > index;
    vector<vector<float> > points;
    vector<vector<int> > normalindex;
    vector<vector<float> > vectors;

    float radius = dishradius * dishradius / height;
    float hd = dishradius;
    int csides = int(2*M_PI * radius / maxSideSize);
    if (csides < minSides) {
        csides = minSides;
    }
    int sides = csides / 2;
    float angle = asin(1-height/radius);

    // Vertexes and normals
    vector<float> v(3, 0);
    vector<float> n(3, 0);
    for (int i = 0; i < sides; i++) {
        float c = (float)cos(angle + (M_PI/2 - angle) / sides * i);
        float s = (float)sin(angle + (M_PI/2 - angle) / sides * i);
        for (int j = 0; j < csides; j++) {
            float C = (float)cos(2*M_PI / csides * j);
            float S = (float)sin(2*M_PI / csides * j);
            v[0] = radius * C * c; v[1] = radius * S * c; v[2] = -(radius - height -radius * s);
            points.push_back(v);
            n[0] = radius * C * c; n[1] = radius * S * c; n[2] = radius * s;
            vectors.push_back(normalize(n));
        }
    }
    v[0] = 0; v[1] = 0; v[2] = height;
    points.push_back(v);
    n[0] = 0; n[1] = 0; n[2] = 1;
    vectors.push_back(n);

    // Sides
    vector<int> pi(3, 0);
    vector<int> ni(3, 0);
    for (int i = 0; i < sides-1; i++) {
        for (int j = 0; j < csides; j++) {
            pi[0] = i*csides+j; pi[2] = i*csides+csides+j; pi[1] = j < csides-1 ? i*csides+1+j : i*csides;
            index.push_back(pi);
            normalindex.push_back(pi);
            pi[0] = i*csides+csides+j; pi[2] = j < csides-1 ? i*csides+csides+1+j : i*csides+csides; pi[1] = j < csides-1 ? i*csides+1+j : i*csides;
            index.push_back(pi);
            normalindex.push_back(pi);
        }
    }
    for (int i = 0; i < csides; i++) {
        pi[0] = csides*(sides-1) + i; pi[1] = i == csides-1 ? csides*(sides-1) : csides*(sides-1) + i+1; pi[2] = points.size()-1;
        index.push_back(pi);
        normalindex.push_back(pi);
    }

    pair<vector<vector<float> >, vector<vector<int> > > vertexes(points, index);
    pair<vector<vector<float> >, vector<vector<int> > > normals(vectors, normalindex);
    return pair<pair<vector<vector<float> >, vector<vector<int> > >, pair<vector<vector<float> >, vector<vector<int> > > >(vertexes, normals);
}

