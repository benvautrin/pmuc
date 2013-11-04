#ifndef RVMCOLORHELPER_H
#define RVMCOLORHELPER_H

#include <vector>

class RVMColorHelper
{
    public:
        RVMColorHelper();

        /**
         * @brief Simple static method to return rgb floats from a PDMS color index. Use the conversion values from Navisworks
         * @param index the PDMS material index
         * @return RGB floats between 0. and 1.
         */
        static const std::vector<float> color(unsigned char index);
};

#endif // RVMCOLORHELPER_H
