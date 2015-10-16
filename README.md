Plant Mock-Up Converter
=======================

Copyright © EDF 2013-2015

About
-----

This project contains a reading library for the RVM file format and a command line utility that allows to convert RVM files to 
- [X3D](http://www.web3d.org/x3d/what-x3d) in XML and binary (FI) encoding,
- [COLLADA](https://www.khronos.org/collada/) files,
- DSL3D language commands, and
- [IFC 2x3](http://www.buildingsmart-tech.org/ifc/IFC2x3/TC1/html/).

License (LGPL)
-------
This library is free software; you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License as published by the Free Software Foundation; either version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with this library; if not, write to the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
  
See the LICENSE.txt file for details.

## How to Build

### Prerequisites

On Unix and Linux, a g++ compiler >= 4.8 is requires.
On Windows, you need at leas Visual Studio 2015. Other platforms / compilers / IDEs have not been tested but are likely to work as well (pls file an issue if not).

[biicode](https://www.biicode.com/) is required to download pmuc dependent libraries. Biicode also requires a [cmake](http://www.cmake.org/) version < 3.0. 

On some platforms, all prequistes for biicode can be install running:
    bii setup:cpp

### Dependencies

Most dependencies are managed by biicode ([eigen](http://eigen.tuxfamily.org), [OpenCOLLADA](https://collada.org/mediawiki/index.php/OpenCOLLADA), [xiot](https://github.com/Supporting/xiot), [IFCPlusPlus](https://github.com/ifcquery/ifcplusplus)) including their subdependencies. Soley a biicode package for libiconv is missing, thus we deliver windows versions with the pmuc repository (contrib folder). On Linux, the functionality of libiconv should be available in standrd libraries (when build-essentials have been installed). If not, try:

    sudo apt-get install libiconv-hook-dev

<!-- An optional dependency also exists on iconv for encoding translations, Windows users can find it at : http://gnuwin32.sourceforge.net/packages/libiconv.htm -->


### Run the build

To start the build, run

    bii buzz

from the root directory.

Credit
------
This work has been led and founded by EDF DIN.
It has been based on an analysis made by Kristian Sons from Supporting GmbH.

See the AUTHORS.txt file for details.




