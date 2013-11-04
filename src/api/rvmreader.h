#ifndef RVMREADER_H
#define RVMREADER_H

#include <string>
#include <vector>

#include "vector3f.h"

/**
 * @brief RVM reader base class
 *
 * Implement this class to use the data found by the RVM parser.
 * Returns data as described in the document written by Kristian Sons
 *
 * @see X3DConverter @see DummyReader
 */
class RVMReader
{
    public:
        /**
         * @brief Default constructor. Initializes variables.
         */
        RVMReader();
        /**
         * @brief Pure virtual destructor
         */
        virtual ~RVMReader() = 0;

        /**
         * @brief Signals the start of the document.
         */
        virtual void startDocument() = 0;
        /**
         * @brief Signals the end of the document.
         */
        virtual void endDocument() = 0;

        /**
         * @brief Called when the header of the RVM file is found
         * @param banner Name and version of PDMS that produced the RVM data.
         * @param fileNote notes concerning the file
         * @param date string describing the date of export.
         * @param user the login of the user that created the data.
         * @param encoding only in version 2 files, string encoding.
         */
        virtual void startHeader(const std::string& banner, const std::string& fileNote, const std::string& date, const std::string& user, const std::string& encoding) = 0;
        /**
         * @brief Called at the end of the header.
         */
        virtual void endHeader() = 0;

        /**
         * @brief Called at the start of the model
         * @param projectName the name of the project
         * @param name the name of the model.
         */
        virtual void startModel(const std::string& projectName, const std::string& name) = 0;
        /**
         * @brief Called at the end of the model.
         */
        virtual void endModel() = 0;

        /**
         * @brief Called at the start of a RVM group
         * @param name the name of the group
         * @param translation the translation of the group, relative to the model origin.
         * @param materialId the material to use in the form of a PDMS color index.
         * @see RVMColorHelper
         */
        virtual void startGroup(const std::string& name, const std::vector<float>& translation, const int& materialId) = 0;
        /**
         * @brief Called at the end of a group.
         */
        virtual void endGroup() = 0;

        /**
         * @brief Called if attributes are found for an group.
         */
        virtual void startMetaData() = 0;
        /**
         * @brief Called at the end of the attributes.
         */
        virtual void endMetaData() = 0;

        /**
         * @brief Called for each key/value attribute pair
         * @param name the name of the attribute.
         * @param value its value.
         */
        virtual void startMetaDataPair(const std::string& name, const std::string& value) = 0;
        /**
         * @brief Called at the end of an attribute.
         */
        virtual void endMetaDataPair() = 0;

        /**
         * @brief Describes a pyramid in a group
         * @param matrix 3x4 transformation matrix
         * @param xbottom
         * @param ybottom
         * @param xtop
         * @param ytop
         * @param height
         * @param xoffset
         * @param yoffset
         * @see RVMMeshHelper::makePyramid
         */
        virtual void startPyramid(const std::vector<float>& matrix,
                                  const float& xbottom,
                                  const float& ybottom,
                                  const float& xtop,
                                  const float& ytop,
                                  const float& height,
                                  const float& xoffset,
                                  const float& yoffset) = 0;
        /**
         * @brief End of a pyramid
         */
        virtual void endPyramid() = 0;

        /**
         * @brief Describes a box in a group
         * @param matrix 3x4 transformation matrix
         * @param xlength
         * @param ylength
         * @param zlength
         * @see RVMMeshHelper::makeBox
         */
        virtual void startBox(const std::vector<float>& matrix,
                              const float& xlength,
                              const float& ylength,
                              const float& zlength) = 0;
        /**
         * @brief End of a box
         */
        virtual void endBox() = 0;

        /**
         * @brief Describes a rectangular torus
         * @param matrix 3x4 transformation matrix
         * @param rinside
         * @param routside
         * @param height
         * @param angle
         * @see RVMMeshHelper::makeRectangularTorus
         */
        virtual void startRectangularTorus(const std::vector<float>& matrix,
                                           const float& rinside,
                                           const float& routside,
                                           const float& height,
                                           const float& angle) = 0;
        /**
         * @brief End of a rectangular torus
         */
        virtual void endRectangularTorus() = 0;

        /**
         * @brief Describes a circular torus
         * @param matrix
         * @param rinside
         * @param routside
         * @param angle
         * @see RVMMeshHelper::makeCircularTorus
         */
        virtual void startCircularTorus(const std::vector<float>& matrix,
                                        const float& rinside,
                                        const float& routside,
                                        const float& angle) = 0;
        /**
         * @brief End of circular torus
         */
        virtual void endCircularTorus() = 0;

        /**
         * @brief Describes an elliptical dish
         * @param matrix
         * @param diameter
         * @param radius
         * @see RVMMeshHelper::makeEllipticalDish
         */
        virtual void startEllipticalDish(const std::vector<float>& matrix,
                                         const float& diameter,
                                         const float& radius) = 0;
        /**
         * @brief End of elliptical dish
         */
        virtual void endEllipticalDish() = 0;

        /**
         * @brief Describes an spherical dish
         * @param matrix
         * @param diameter
         * @param height
         * @see RVMMeshHelper::makeSphericalDish
         */
        virtual void startSphericalDish(const std::vector<float>& matrix,
                                        const float& diameter,
                                        const float& height) = 0;
        /**
         * @brief End of a spherical dish.
         */
        virtual void endSphericalDish() = 0;

        /**
         * @brief Describes a snout.
         * @param matrix
         * @param dtop
         * @param dbottom
         * @param height
         * @param xoffset
         * @param yoffset
         * @param unknown1
         * @param unknown2
         * @param unknown3
         * @param unknown4
         * @see RVMMeshHelper::makeSnout
         */
        virtual void startSnout(const std::vector<float>& matrix,
                                const float& dtop,
                                const float& dbottom,
                                const float& height,
                                const float& xoffset,
                                const float& yoffset,
                                const float& unknown1,
                                const float& unknown2,
                                const float& unknown3,
                                const float& unknown4) = 0;
        /**
         * @brief End of a snout.
         */
        virtual void endSnout() = 0;

        /**
         * @brief Describes a cylinder
         * @param matrix
         * @param diameter
         * @param height
         * @see RVMMeshHelper::makeCylinder
         */
        virtual void startCylinder(const std::vector<float>& matrix,
                                   const float& diameter,
                                   const float& height) = 0;
        /**
         * @brief End of a cylinder
         */
        virtual void endCylinder() = 0;

        /**
         * @brief Describes a sphere
         * @param matrix
         * @param diameter
         * @see RVMMeshHelper::makeSphere
         */
        virtual void startSphere(const std::vector<float>& matrix,
                                 const float& diameter) = 0;
        /**
         * @brief End of a sphere
         */
        virtual void endSphere() = 0;

        /**
         * @brief Describes a line
         * @param matrix
         * @param startx
         * @param endx
         */
        virtual void startLine(const std::vector<float>& matrix,
                               const float& startx,
                               const float& endx) = 0;
        /**
         * @brief End of a line
         */
        virtual void endLine() = 0;

        /**
         * @brief Describes a facet group
         *
         * Separated in patch/group/vertexes.
         * If more than one group is found in a patch, each group should be closed.
         *
         * @param matrix
         * @param vertexes
         */
        virtual void startFacetGroup(const std::vector<float>& matrix,
                                     const std::vector<std::vector<std::vector<std::pair<Vector3F, Vector3F> > > >& vertexes) = 0;
        /**
         * @brief End of a facet group
         */
        virtual void endFacetGroup() = 0;

        /**
         * @brief Sets the maximum size for a side of a primitive when tesselating.
         * @param size
         */
        void setMaxSideSize(float size) { m_maxSideSize = size; }
        /**
         * @brief Sets the minimum number of sides of a tesselated primitive.
         * @param number
         */
        void setMinSides(int number) { m_minSides = number; }
        /**
         * @brief Sets if the user wants the data to be split with a file for each group.
         * @param split
         */
        void setSplit(bool split) { m_split = split; }
        /**
         * @brief Sets if the reader should use native primitives instead of tesselating.
         * @param primitives
         */
        void setUsePrimitives(bool primitives) { m_primitives = primitives; }

    protected:
        int m_minSides;
        float m_maxSideSize;
        bool m_split;
        bool m_primitives;
};

#endif // RVMREADER_H
