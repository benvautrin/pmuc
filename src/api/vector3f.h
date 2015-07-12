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

#ifndef Vector3F_H
#define Vector3F_H

#include <vector>
#include <ostream>
#include <cmath>

/**
 * @brief Class representing a 3D vector or point.
 *
 * Optimized for quick allocation. Allows the use of operators such as -, +, *...
 */
class Vector3F
{
    public:
        Vector3F();
        Vector3F(const float& x, const float& y, const float& z);
        Vector3F(const Vector3F& v);
        Vector3F(const std::vector<float>& v);

        inline float& operator[](int i) {
            return m_values[i];
        }
        inline const float& operator[](int i) const {
            return m_values[i];
        }
        inline float& x() {
            return m_values[0];
        }
        inline const float& x() const {
            return m_values[0];
        }
        inline float& y() {
            return m_values[1];
        }
        inline const float& y() const {
            return m_values[1];
        }
        inline float& z() {
            return m_values[2];
        }
        inline const float& z() const {
            return m_values[2];
        }

        float squaredNorm() const { return x()*x() + y()*y() + z()*z(); }

        float normalize() {
            const float n = squaredNorm();
            if(n != 0) {
                float mag = 1.0f / sqrt(n);
                m_values[0] *= mag;
                m_values[1] *= mag;
                m_values[2] *= mag;
                return mag;
            }
            return 0.f;
        }

        inline bool equals(const Vector3F& v) const {
            return (m_values[0] == v[0]) && (m_values[1] == v[1]) && (m_values[2] == v[2]);
        }

         Vector3F& operator*=(float k)  {
            m_values[0] *= k; m_values[1] *= k; m_values[2] *= k;
            return *this;
        }

        friend Vector3F operator-(const std::vector<float>& p1, const std::vector<float>& p2);
        friend Vector3F operator-(const Vector3F& p1, const Vector3F& p2);
        friend Vector3F operator+(const Vector3F& p1, const Vector3F& p2);
        friend float operator*(const Vector3F& p1, const Vector3F& p2);
        friend Vector3F operator*(const Vector3F& v, float f);
        friend std::ostream& operator<<(std::ostream& out, const Vector3F& vec);

        friend bool operator!=(const Vector3F &a, const Vector3F &b) {
            return !(a==b);
        };

        friend bool operator==(const Vector3F &a, const Vector3F &b) {
           const float epsilon = 1.0E-10f;
           return (a-b).squaredNorm() < epsilon;
        }

    private:
        float m_values[3];
};



#endif // Vector3F_H
