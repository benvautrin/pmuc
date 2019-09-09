# Plant Mock-Up Converter

[![Build Status](https://api.travis-ci.org/Supporting/pmuc.svg?branch=master)](https://travis-ci.org/Supporting/pmuc)
[![Build status](https://ci.appveyor.com/api/projects/status/pt0xdf8srhyui2gc/branch/master?svg=true)](https://ci.appveyor.com/project/Supporting/pmuc/branch/master)

Copyright © EDF 2013-2016

## About

This project contains a reading library for the RVM file format and a command line utility that allows to convert RVM files to

- [X3D](http://www.web3d.org/x3d/what-x3d) in XML and binary (FI) encoding,
- [COLLADA](https://www.khronos.org/collada/) files,
- DSL3D language commands, and
- [IFC 2x3](http://www.buildingsmart-tech.org/ifc/IFC2x3/TC1/html/).

## License (LGPL)

This library is free software; you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License as published by the Free Software Foundation; either version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with this library; if not, write to the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA

See the LICENSE.txt file for details.

## How to Build

### Prerequisites

On Unix and Linux, a g++ compiler >= 4.8 is required.
On Windows, you need at least Visual Studio 2012. Other platforms / compilers / IDEs have not been tested but are likely to work as well (please file an issue if not).

Building requires a [cmake](http://www.cmake.org/) version >= 2.8.

### Dependencies

Most dependencies are included in git submodules, namely [eigen](http://eigen.tuxfamily.org), [OpenCOLLADA](https://collada.org/mediawiki/index.php/OpenCOLLADA), and [xiot](https://github.com/Supporting/xiot). To inititalize the submodules, run:

    git submodule init
    git submodule update

Soley [boost](http://www.boost.org/) needs to be provided externally. On Linux it is sufficient to install the boost development package, e.g.:

    sudo apt-get install libboost-all-dev

On other systems, let the environment varaible ``BOOST_ROOT```point to Boost root directory.

### Run the build

#### Windows

Run cmake from a different directory than your source (e.g. in a `build` folder) and select the generator of your choice, e.g.:

    cmake -G "Visual Studio 14 2015 Win64" ..

Open the generated IDE file (pmuc.sln) or start the build from command line:

    cmake --build . --target pmuc --config Release

#### Linux

Run:

    mkdir build && cd build && cmake .. -DCMAKE_BUILD_TYPE=Release
    cmake --build . --target pmuc

### Running Tests

PMUC includes a number of running tests that can be run from the build directory using `ctest`:

    ctest -C Release --output-on-failure

## Credit

This work has been led and founded by EDF DIN.
It has been based on an analysis made by Kristian Sons from Supporting GmbH.

See the AUTHORS.txt file for details.
