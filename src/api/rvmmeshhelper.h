#ifndef RVMMESHHELPER_H
#define RVMMESHHELPER_H

#include <utility>
#include <vector>

class RVMMeshHelper
{
    public:
        RVMMeshHelper();

    public:
        /**
         * @brief Builds up indexed coordinates for the described pyramid
         * @param xbottom
         * @param ybottom
         * @param xtop
         * @param ytop
         * @param xoffset
         * @param yoffset
         * @param height
         * @param maxSideSize not used here. For consistency with the other methods.
         * @param minSides not used here. For consistency with the other methods.
         * @return vertexes coordinates and their index.
         */
        static const std::pair<std::vector<std::vector<float> >, std::vector<std::vector<int> > > makePyramid(const float& xbottom, const float& ybottom, const float& xtop, const float& ytop, const float& xoffset, const float& yoffset, const float& height, const float& maxSideSize, const int& minSides);

        /**
         * @brief Builds up indexed coordinates for the described box
         * @param x
         * @param y
         * @param z
         * @param maxSideSize not used here. For consistency with the other methods.
         * @param minSides not used here. For consistency with the other methods.
         * @return vertexes coordinates and their index.
         */
        static const std::pair<std::vector<std::vector<float> >, std::vector<std::vector<int> > > makeBox(const float& x, const float& y, const float& z, const float& maxSideSize, const int& minSides);

        /**
         * @brief Builds up a sphere with the given radius
         * @param radius
         * @param maxSideSize
         * @param minSides
         * @return coordinates and normals with their indexes.
         */
        static const std::pair<std::pair<std::vector<std::vector<float> >, std::vector<std::vector<int> > >, std::pair<std::vector<std::vector<float> >, std::vector<std::vector<int> > > > makeSphere(const float& radius, const float& maxSideSize, const int& minSides);

        /**
         * @brief makeCylinder
         * @param radius
         * @param height
         * @param maxSideSize
         * @param minSides
         * @return
         */
        static const std::pair<std::pair<std::vector<std::vector<float> >, std::vector<std::vector<int> > >, std::pair<std::vector<std::vector<float> >, std::vector<std::vector<int> > > > makeCylinder(const float& radius, const float& height, const float& maxSideSize, const int& minSides);

        /**
         * @brief makeRectangularTorus
         * @param rinside
         * @param routside
         * @param height
         * @param angle
         * @param maxSideSize
         * @param minSides
         * @return
         */
        static const std::pair<std::pair<std::vector<std::vector<float> >, std::vector<std::vector<int> > >, std::pair<std::vector<std::vector<float> >, std::vector<std::vector<int> > > > makeRectangularTorus(const float& rinside, const float& routside, const float& height, const float& angle, const float& maxSideSize, const int& minSides);

        /**
         * @brief makeCircularTorus
         * @param rinside
         * @param routside
         * @param angle
         * @param maxSideSize
         * @param minSides
         * @return
         */
        static const std::pair<std::pair<std::vector<std::vector<float> >, std::vector<std::vector<int> > >, std::pair<std::vector<std::vector<float> >, std::vector<std::vector<int> > > > makeCircularTorus(const float& rinside, const float& routside, const float& angle, const float& maxSideSize, const int& minSides);

        /**
         * @brief makeSnout
         * @param rbottom
         * @param rtop
         * @param height
         * @param xoffset
         * @param yoffset
         * @param maxSideSize
         * @param minSides
         * @return
         */
        static const std::pair<std::pair<std::vector<std::vector<float> >, std::vector<std::vector<int> > >, std::pair<std::vector<std::vector<float> >, std::vector<std::vector<int> > > > makeSnout(const float& rbottom, const float& rtop, const float& height, const float& xoffset, const float& yoffset, const float& maxSideSize, const int& minSides);

        /**
         * @brief makeEllipticalDish
         * @param diameter
         * @param radius
         * @param maxSideSize
         * @param minSides
         * @return
         */
        static const std::pair<std::pair<std::vector<std::vector<float> >, std::vector<std::vector<int> > >, std::pair<std::vector<std::vector<float> >, std::vector<std::vector<int> > > > makeEllipticalDish(const float& diameter, const float& radius, const float& maxSideSize, const int& minSides);

        /**
         * @brief makeSphericalDish
         * @param diameter
         * @param height
         * @param maxSideSize
         * @param minSides
         * @return
         */
        static const std::pair<std::pair<std::vector<std::vector<float> >, std::vector<std::vector<int> > >, std::pair<std::vector<std::vector<float> >, std::vector<std::vector<int> > > > makeSphericalDish(const float& diameter, const float& height, const float& maxSideSize, const int& minSides);
};

#endif // RVMMESHHELPER_H
