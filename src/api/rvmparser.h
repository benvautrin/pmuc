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

#ifndef RVMPARSER_H
#define RVMPARSER_H

#include <istream>
#include <string>
#include <vector>
#include <array>
#include <map>
#include <list>

#include "vector3f.h"
#include "lib_export.h"

class RVMReader;

/**
 * @brief The RVMParser class
 * This is a parser class able to read RVM data from either:
 *   - a file,
 *   - a character buffer,
 *   - or a standard input stream.
 *
 * In the case of a file, a .att companion file will be searched int he same directory to include metadata.
 *
 * Two methods are provided to allow tweeking the output:
 * @see setObjectName @see setForcedColor
 */
class RVMParser
{
    public:
        /**
         * @brief Constructs a parser ready to send data to the provided RVMReader
         * @param reader The reader object that will receive the data.
         */
        DLL_PMUC_EXPORT RVMParser(RVMReader& reader);

        /**
         * @brief Reads from a file a parse its content.
         * @param filename the file name
         *
         * @return true if the parsing was a success.
         */
        DLL_PMUC_EXPORT bool readFile(const std::string& filename, bool ignoreAttributes);
        /**
         * @brief Reads from a series of files.
         * @param filenames a vector of filenames
         *
         * @return true if the parsing was a success.
         */
        DLL_PMUC_EXPORT bool readFiles(const std::vector<std::string>& filenames, const std::string& name, bool ignoreAttributes);
        /**
         * @brief Reads from a character buffer.
         * @param buffer the character buffer containing RVM data.
         * @return true if the parsing was a success.
         */
        DLL_PMUC_EXPORT bool readBuffer(const char* buffer);
        /**
         * @brief Reads RVM data from an input stream
         * @param is the input stream of RVM data.
         * @return true if the parsing was a success.
         */
        DLL_PMUC_EXPORT bool readStream(std::istream& is, std::istream* attributeStream = NULL);

        /**
         * @brief Allow to filter the RVM data to one named object
         * @param name the name of the object to extract.
         */
        void setObjectName(const std::string& name) { m_objectName = name; }
        /**
         * @brief Force the color of all extracted objects
         * @param index a PDMS color index
         */
        void setForcedColor(const int index) { m_forcedColor = index; }
        void setScale(const float scale) { m_scale = scale; }

        /**
         * @brief In case of error, returns the last error that occured.
         * @return a string describing the error.
         */
        DLL_PMUC_EXPORT const std::string lastError();

        /**
         * @brief Statistics of the parsing: number of groups
         * @return the number of groups found in the source.
         */
        const int& nbGroups() { return m_nbGroups; }
        /**
         * @brief Statistics of the parsing: number of pyramids
         * @return the number of pyramids found in the source.
         */
        const int& nbPyramids() { return m_nbPyramids; }
        /**
         * @brief Statistics of the parsing: number of boxes
         * @return the number of boxes found in the source.
         */
        const int& nbBoxes() { return m_nbBoxes; }
        /**
         * @brief Statistics of the parsing: number of rectangular toruses
         * @return the number of rectangular toruses found in the source.
         */
        const int& nbRectangularToruses() { return m_nbRectangularToruses; }
        /**
         * @brief Statistics of the parsing: number of circular toruses
         * @return the number of circular toruses found in the source.
         */
        const int& nbCircularToruses() { return m_nbCircularToruses; }
        /**
         * @brief Statistics of the parsing: number of elliptical dishes
         * @return the number of elliptical dishes found in the source.
         */
        const int& nbEllipticalDishes() { return m_nbEllipticalDishes; }
        /**
         * @brief Statistics of the parsing: number of spherical dishes
         * @return the number of spherical dishes found in the source.
         */
        const int& nbSphericalDishes() { return m_nbSphericalDishes; }
        /**
         * @brief Statistics of the parsing: number of snouts
         * @return the number of snouts found in the source.
         */
        const int& nbSnouts() { return m_nbSnouts; }
        /**
         * @brief Statistics of the parsing: number of cylinders
         * @return the number of cylinders found in the source.
         */
        const int& nbCylinders() { return m_nbCylinders; }
        /**
         * @brief Statistics of the parsing: number of spheres
         * @return the number of spheres found in the source.
         */
        const int& nbSpheres() { return m_nbSpheres; }
        /**
         * @brief Statistics of the parsing: number of lines
         * @return the number of lines found in the source.
         */
        const int& nbLines() { return m_nbLines; }
        /**
         * @brief Statistics of the parsing: number of facet groups
         * @return the number of facet groups found in the source.
         */
        const int& nbFacetGroups() { return m_nbFacetGroups; }
        /**
         * @brief Statistics of the parsing: number of attributes
         * @return the number of attributes found in the source.
         */
        const long& nbAttributes() { return m_attributes; }
       
        /**
         * @brief Stop reading file/stream by user
         */
        DLL_PMUC_EXPORT void Break() { m_brokenByUser = true; }

    private:
        bool readGroup(std::istream& is, std::istream* attributeStream);
        bool readPrimitive(std::istream& is);
        bool readColor(std::istream& is);
        void readArttributes(std::istream& stream);
        void insertAttributes(const std::string& name, bool ignoreAttributes);
        bool isFound(const std::string& line, const std::string& substring, size_t& after);


        void readMatrix(std::istream& is, std::array<float, 12>& matrix);

        RVMReader       &m_reader;
        std::string     m_encoding;
        std::string     m_lastError;

        std::string     m_objectName;
        int             m_objectFound;
        int             m_forcedColor;
        bool            m_aggregation;
        float           m_scale;

        int             m_nbGroups;
        int             m_nbPyramids;
        int             m_nbBoxes;
        int             m_nbRectangularToruses;
        int             m_nbCircularToruses;
        int             m_nbEllipticalDishes;
        int             m_nbSphericalDishes;
        int             m_nbSnouts;
        int             m_nbCylinders;
        int             m_nbSpheres;
        int             m_nbLines;
        int             m_nbFacetGroups;
        long            m_attributes;

        bool            m_brokenByUser;

        using TAttributePair  = std::pair<std::string, std::string>;
        using TAttributeContainer = std::list<TAttributePair>;

        std::map<std::string, TAttributeContainer> m_groupAttributes;
};

#endif // RVMPARSER_H
