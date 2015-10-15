#ifndef RVMPRIMITVE_H
#define RVMPRIMITVE_H

namespace Primitives
{
    struct Box
    {
        float       len[3];
    };

    struct Pyramid
    {
        inline float& xbottom() { return data[0]; }
        inline const float& xbottom() const { return data[0]; }
        inline float& ybottom() { return data[1]; }
        inline const float& ybottom() const { return data[1]; }

        inline float& xtop() { return data[2]; }
        inline const float& xtop() const { return data[2]; }
        inline float& ytop() { return data[3]; }
        inline const float& ytop() const { return data[3]; }

        inline float& xoffset() { return data[4]; }
        inline const float& xoffset() const { return data[4]; }
        inline float& yoffset() { return data[5]; }
        inline const float& yoffset() const { return data[5]; }

        inline float& height() { return data[6]; }
        inline const float& height() const { return data[6]; }

        float       data[7]; // xbottom, ybottom, xtop, ytop, height, xoffset, yoffset
    };

    struct RectangularTorus
    {
        inline float& rinside() { return data[0]; }
        inline const float& rinside() const { return data[0]; }
        inline float& routside() { return data[1]; }
        inline const float& routside() const { return data[1]; }
        inline float& height() { return data[2]; }
        inline const float& height() const { return data[2]; }
        inline float& angle() { return data[3]; }
        inline const float& angle() const { return data[3]; }

        float       data[4];
    };

    struct CircularTorus
    {
        inline float& rinside() { return data[0]; }
        inline const float& rinside() const { return data[0]; }
        inline float& routside() { return data[1]; }
        inline const float& routside() const { return data[1]; }
        inline float& angle() { return data[2]; }
        inline const float& angle() const { return data[2]; }

        float       data[3];
    };

    struct EllipticalDish
    {
        inline float& diameter() { return data[0]; }
        inline const float& diameter() const { return data[0]; }
        inline float& radius() { return data[1]; }
        inline const float& radius() const { return data[1]; }

        float       data[2];
    };

    struct SphericalDish
    {
        inline float& diameter() { return data[0]; }
        inline const float& diameter() const { return data[0]; }
        inline float& height() { return data[1]; }
        inline const float& height() const { return data[1]; }

        float       data[2];
    };

    struct Snout
    {
        inline float& dbottom() { return data[0]; }
        inline const float& dbottom() const { return data[0]; }
        inline float& dtop() { return data[1]; }
        inline const float& dtop() const { return data[1]; }
        inline float& height() { return data[2]; }
        inline const float& height() const { return data[2]; }
        inline float& xoffset() { return data[3]; }
        inline const float& xoffset() const { return data[3]; }
        inline float& yoffset() { return data[4]; }
        inline const float& yoffset() const { return data[4]; }

        float       data[5];
    };

    struct Cylinder
    {
        inline float& radius() { return data[0]; }
        inline const float& radius() const { return data[0]; }
        inline float& height() { return data[1]; }
        inline const float& height() const { return data[1]; }

        float       data[2];
    };

    struct Sphere
    {
        float           diamater;
    };
}

#endif // RVMPRIMITVE_H
