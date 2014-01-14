Plant Mock-Up Converter
=======================

Copyright © EDF 2013

About
-----

This project contains a reading library for the RVM file format and a command line utility that allows to convert RVM files to X3D files (XML or binary), Collada files or DSL3D language commands.

License
-------
This library is free software; you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License as published by the Free Software Foundation; either version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with this library; if not, write to the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
  
See the LICENSE.txt file for details.

Credit
------
This work has been led and founded by EDF DIN.
It has been based on an analysis made by Kristian Sons from Supporting GmbH.

See the AUTHORS.txt file for details.

Dependencies
------------

The library itself only relies on the C++ standard library but the converter uses several open-source projects:
 - XIOT, a X3D/X3DB input/ouput library, https://github.com/Supporting/xiot
 - OpenCOLLADA, and more precisely its StreamWriter library, https://collada.org/mediawiki/index.php/OpenCOLLADA
 - Eigen, a C++ template library for linear algebra, http://eigen.tuxfamily.org

An optional dependency also exists on iconv for encoding translations, Windows users can find it at : http://gnuwin32.sourceforge.net/packages/libiconv.htm


