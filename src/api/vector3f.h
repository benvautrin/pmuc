#ifndef Vector3F_H
#define Vector3F_H

#include <vector>

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

        inline bool equals(const Vector3F& v) const {
            return (m_values[0] == v[0] && m_values[1] == v[1] && m_values[2] == v[2]);
        }

    private:
        float m_values[3];
};


Vector3F operator-(const std::vector<float>& p1, const std::vector<float>& p2);

Vector3F operator-(const Vector3F& p1, const Vector3F& p2);

Vector3F operator+(const Vector3F& p1, const Vector3F& p2);

float operator*(const Vector3F& p1, const Vector3F& p2);

Vector3F operator*(const Vector3F& v, float f);

std::ostream& operator<<(std::ostream& out, const Vector3F& vec);

#endif // Vector3F_H
