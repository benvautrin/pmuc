#include "vector3f.h"

#include <iostream>

Vector3F::Vector3F() {
    m_values[0] = 0;
    m_values[1] = 0;
    m_values[2] = 0;
}

Vector3F::Vector3F(const float& x, const float& y, const float& z) {
    m_values[0] = x;
    m_values[1] = y;
    m_values[2] = z;
}

Vector3F::Vector3F(const Vector3F& v) {
    m_values[0] = v[0];
    m_values[1] = v[1];
    m_values[2] = v[2];
}

Vector3F::Vector3F(const std::vector<float>& v) {
    m_values[0] = v[0];
    m_values[1] = v[1];
    m_values[2] = v[2];
}

Vector3F operator-(const std::vector<float>& p1, const std::vector<float>& p2) {
    Vector3F d;
    d[0] = p1[0] - p2[0];
    d[1] = p1[1] - p2[1];
    d[2] = p1[2] - p2[2];
    return d;
}

Vector3F operator-(const Vector3F& p1, const Vector3F& p2) {
    Vector3F d;
    d[0] = p1[0] - p2[0];
    d[1] = p1[1] - p2[1];
    d[2] = p1[2] - p2[2];
    return d;
}

Vector3F operator+(const Vector3F& p1, const Vector3F& p2) {
    Vector3F d;
    d[0] = p1[0] + p2[0];
    d[1] = p1[1] + p2[1];
    d[2] = p1[2] + p2[2];
    return d;
}

float operator*(const Vector3F& p1, const Vector3F& p2) {
    return p1[0] * p2[0] + p1[1] * p2[1] + p1[2] * p2[2];
}

Vector3F operator*(const Vector3F& v, float f) {
    Vector3F r;
    r[0] = v[0] * f;
    r[1] = v[1] * f;
    r[2] = v[2] * f;
    return r;
}

std::ostream& operator<<(std::ostream& out, const Vector3F& vec) {
    out << "(" << vec.x() << ", " << vec.y() << ", " << vec.z() << ")";
    return out;
}
