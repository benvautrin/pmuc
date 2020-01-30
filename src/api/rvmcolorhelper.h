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

#ifndef RVMCOLORHELPER_H
#define RVMCOLORHELPER_H

#include "lib_export.h"

#include <vector>

class RVMColorHelper
{
    public:
        DLL_PMUC_EXPORT RVMColorHelper();

        /**
         * @brief Simple static method to return rgb floats from a PDMS color index. Use the conversion values from Navisworks
         * @param index the PDMS material index
         * @return RGB floats between 0. and 1.
         */
        DLL_PMUC_EXPORT static const std::vector<float> color(unsigned char index);
};

#endif // RVMCOLORHELPER_H
