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

#include <ctime>

#include <iostream>
#include <cstdlib>

#include "optionparser.h"
#include "api/rvmparser.h"
#include "converters/dummyreader.h"
#include "converters/dslconverter.h"

#ifdef XIOT_FOUND
#include "converters/x3dconverter.h"
#endif // XIOT_FOUND

#ifdef OPENCOLLADASW_FOUND
#include "converters/colladaconverter.h"
#endif // OPENCOLLADASW_FOUND

#ifdef _MSC_VER
#define _USE_MATH_DEFINES // For PI under VC++
#endif

#include <math.h>

#ifdef _WIN32
#define PATHSEP '\\'
#else
#define PATHSEP '/'
#endif

using namespace std;

enum optionIndex { UNKNOWN,
                   HELP,
                   TEST,
#ifdef XIOT_FOUND
                   X3D,
                   X3DB,
#endif // XIOT_FOUND
#ifdef OPENCOLLADASW_FOUND
                   COLLADA,
#endif // OPENCOLLADASW_FOUND
                   DSL,
                   DUMMY,
                   SKIPATT,
                   SPLIT,
                   AGGREGATE,
                   PRIMITIVES,
                   SIDESIZE,
                   MINSIDES,
                   OBJECT,
                   COLOR,
                   SCALE };
const option::Descriptor usage[] = {
    { UNKNOWN,      0, "",  "",              option::Arg::None,      "\nusage: pmuc [options] <rvm file 1> ...\n\nChoose at least one format and one file to convert.\nOptions:" },
    { HELP,         0, "h", "help",          option::Arg::None,      "  --help, -h \tPrint usage and exit." },
#ifdef XIOT_FOUND
    { X3D,          0, "",  "x3d",           option::Arg::None,      "  --x3d  \tConvert to X3D XML format." },
    { X3DB,         0, "",  "x3db",          option::Arg::None,      "  --x3db  \tConvert to X3D binary format." },
#endif // XIOT_FOUND
#ifdef OPENCOLLADASW_FOUND
    { COLLADA,      0, "",  "collada",       option::Arg::None,      "  --collada\tConvert to COLLADA format." },
#endif // OPENCOLLADASW_FOUND
    { DSL,          0, "",  "dsl",           option::Arg::None,      "  --dsl  \tConvert to DSL language." },
    { DUMMY,        0, "",  "dummy",         option::Arg::None,      "  --dummy\tPrint out the file structure." },
    { SKIPATT,      0, "",  "skipattributes",option::Arg::None,      "  --skipattributes \tIgnore attribute file." },
    { SPLIT,        0, "",  "split",         option::Arg::None,      "  --split \tIf possible split in sub files (Only X3D)." },
    { AGGREGATE,    0, "",  "aggregate",     option::Arg::Optional,  "  --aggregate=<name> \tCombine input files in one export file." },
    { PRIMITIVES,   0, "",  "primitives",    option::Arg::None,      "  --primitives  \tIf possible use native primitives." },
    { SIDESIZE ,    0, "",  "maxsidesize",   option::Arg::Optional,  "  --maxsidesize=<length>  \tUsed for tesselation. Default 1000." },
    { MINSIDES ,    0, "",  "minsides",      option::Arg::Optional,  "  --minsides=<nb>  \tUsed for tesselation. Default 8." },
    { TEST,         0, "t", "test",          option::Arg::None,      "  --test, -t \tOutputs primitive samples for testing purposes." },
    { OBJECT,       0, "",  "object",        option::Arg::Optional,  "  --object=<name> \tExtract only the named object." },
    { COLOR,        0, "",  "color",         option::Arg::Optional,  "  --color=<index> \tForce a PDMS color on all objects." },
    { SCALE,        0, "",  "scale",         option::Arg::Optional,  "  --scale=<multiplier> \tScale the model." },
    {0,0,0,0,0,0}
};
const string formatnames[] = {
    "",
    "",
    "",
#ifdef XIOT_FOUND
    "X3D",
    "X3DB",
#endif // XIOT_FOUND
#ifdef OPENCOLLADASW_FOUND
    "COLLADA",
#endif // OPENCOLLADASW_FOUND
    "DSL",
    "DUMMY",
};

enum primitives { BOX, SNOUT, CYLINDER, SPHERE, CIRCULARTORUS, RECTANGULARTORUS, PYRAMID, LINE, ELLIPTICALDISH, SPHERICALDISH };
const string primitiveNames[] = { "box", "snout", "cylinder", "sphere", "circulartorus", "rectangulartorus", "pyramid", "line", "ellipticaldish", "sphericaldish" };

int main(int argc, char** argv)
{
    cout << "Plant Mock-Up Converter 0.1\nCopyright (C) EDF 2013" << endl;

    argc -= (argc > 0); argv += (argc > 0);
    option::Stats stats(usage, argc, argv);
    option::Option* options = new option::Option[stats.options_max];
    option::Option* buffer = new option::Option[stats.buffer_max];
    option::Parser parse(usage, argc, argv, options, buffer);

    if (parse.error()) {
        cout << "error." <<endl;
        return 1;
    }

    if (options[HELP] || argc == 0) {
        option::printUsage(std::cout, usage);
        return 0;
    }

    if ((
#ifdef XIOT_FOUND
         options[X3D] || options[X3DB] ||
#endif // XIOT_FOUND
#ifdef OPENCOLLADASW_FOUND
         options[COLLADA] ||
#endif // OPENCOLLADASW_FOUND
         options[DSL] || options[DUMMY]) == 0) {
        cout << "\nNo format specified.\n";
        option::printUsage(std::cout, usage);
        return 1;
    }

    if (parse.nonOptionsCount() == 0 && options[TEST].count() == 0) {
        cout << "\nNo file specified.\n";
        option::printUsage(std::cout, usage);
        return 1;
    }

    int minSides = 8;
    if (options[MINSIDES].count() > 0) {
        minSides = atoi(options[MINSIDES].arg);
        if (minSides < 5) {
            cout << "\n--minsides option should be > 4.\n";
            option::printUsage(std::cout, usage);
            return 1;
        }
    }

    float maxSideSize = 1000.;
    if (options[SIDESIZE].count() > 0) {
        maxSideSize = (float)atof(options[SIDESIZE].arg);
        if (maxSideSize <= 0) {
            cout << "\n--maxsidesize option should be > 0.\n";
            option::printUsage(std::cout, usage);
            return 1;
        }
    }

    int forcedColor = -1;
    if (options[COLOR].count()) {
        forcedColor = atoi(options[COLOR].arg);
        if (forcedColor < 0 || forcedColor > 255) {
            cout << "\n--color option should be >= 0 and <= 255.\n";
            option::printUsage(std::cout, usage);
            return 1;
        }
    }

    float scale = 1.;
    if (options[SCALE].count()) {
        scale = (float)atof(options[SCALE].arg);
    }

    string objectName = options[OBJECT].count() ? options[OBJECT].arg : "";
    if (!objectName.empty()) {
        size_t p;
        while ((p = objectName.find_first_of(' ')) != string::npos)
            objectName[p] = '_';
        while (objectName[0] == '-')
            objectName[0] = '_';
        while ((p = objectName.find_first_of('/')) != string::npos) {
            objectName[p] = '_';
        }
    }

    // Testing: outputs primitives in individual files.
    if (options[TEST].count() > 0) {
        cout << "\nWriting primitive example files..." << endl;
        for (int i = 0; i < 10; i++) {
            string filename = primitiveNames[i];
            cout << filename << "." << endl;
            for (int format = TEST + 1; format <= DUMMY; format++) {
                if (options[format].count() > 0) {
                    RVMReader* reader;
                    switch (format) {
#ifdef XIOT_FOUND
                        case X3D:
                        case X3DB: {
                            string name = filename + ".x3d" + (format == X3D ? "" : "b");
                            reader = new X3DConverter(name, format == X3DB);
                        } break;
#endif // XIOT_FOUND

#ifdef OPENCOLLADASW_FOUND
                        case COLLADA: {
                            string name = filename + ".dae";
                            reader = new COLLADAConverter(name);
                        } break;
#endif // OPENCOLLADASW_FOUND

                        case DSL: {
                            string name = filename + ".dsl3d";
                            reader = new DSLConverter(name);
                        } break;
                    }
                    if (maxSideSize) {
                        reader->setMaxSideSize(maxSideSize);
                    }
                    if (minSides) {
                        reader->setMinSides(minSides);
                    }
                    reader->setUsePrimitives(options[PRIMITIVES].count() > 0);
                    reader->setSplit(options[SPLIT].count() > 0);
                    vector<float> translation;
                    for (int j = 0; j < 3; j++) translation.push_back(0);
                    vector<float> matrix;
                    for (int j = 0; j < 12; j++) matrix.push_back(0);
                    matrix[0] = 1; matrix[4] = 1; matrix[8] = 1;
                    reader->startDocument();
                    reader->startHeader("Plant Mock-Up Converter", "Primitive example file", "", "", "");
                    reader->endHeader();
                    reader->startModel("Primitive examples", primitiveNames[i]);
                    reader->startGroup(primitiveNames[i], translation, forcedColor != -1 ? forcedColor : 2);
                    switch (i) { // BOX, SNOUT, CYLINDER, SPHERE, CIRCULARTORUS, RECTANGULARTORUS, PYRAMID, LINE, ELLIPTICALDISH, SPHERICALDISH
                        case BOX: {
                            reader->startBox(matrix, 1, 1, 1);
                            reader->endBox();
                        } break;
                        case SNOUT: {
                            reader->startSnout(matrix, 2, 4, 2, 1, 1, 0, 0, 0, 0);
                            reader->endSnout();
                        } break;
                        case CYLINDER: {
                            reader->startCylinder(matrix, 1, 2);
                            reader->endCylinder();
                        } break;
                        case SPHERE: {
                            reader->startSphere(matrix, 2);
                            reader->endSphere();
                        } break;
                        case CIRCULARTORUS: {
                            reader->startCircularTorus(matrix, 4, 2, (float)M_PI);
                            reader->endCircularTorus();
                        } break;
                        case RECTANGULARTORUS: {
                            reader->startRectangularTorus(matrix, 2, 3, 0.5, (float)M_PI*3/4);
                            reader->endRectangularTorus();
                        } break;
                        case PYRAMID: {
                            reader->startPyramid(matrix, 2, 4, 1, 2, 2, 1, 4);
                            reader->endPyramid();
                        } break;
                        case LINE: {
                            reader->startLine(matrix, 1, 2);
                            reader->endLine();
                        } break;
                        case ELLIPTICALDISH: {
                            reader->startEllipticalDish(matrix, 4, 4);
                            reader->endEllipticalDish();
                        } break;
                        case SPHERICALDISH: {
                            reader->startSphericalDish(matrix, 4, 1);
                            reader->endSphericalDish();
                        } break;
                    }
                    reader->endGroup();
                    reader->endModel();
                    reader->endDocument();
                    delete reader;
                }
            }
        }
        cout << "done." << endl;
    }

    // File conversions.
    if (options[AGGREGATE].count() > 0) {
        for (int format = TEST + 1; format <= DUMMY; format++) {
            if (options[format].count() > 0) {
                time_t start = time(0);
                RVMReader* reader;
                string name = options[AGGREGATE].arg;
                switch (format) {
                    case DUMMY: {
                        reader = new DummyReader;
                    } break;

#ifdef XIOT_FOUND
                    case X3D:
                    case X3DB: {
                        string x3dname = name + ".x3d" + (format == X3D ? "" : "b");
                        reader = new X3DConverter(x3dname, format == X3DB);
                    } break;
#endif // XIOT_FOUND

#ifdef OPENCOLLADASW_FOUND
                    case COLLADA: {
                        string colladaname = name + ".dae";
                        reader = new COLLADAConverter(colladaname);
                    } break;
#endif // OPENCOLLADASW_FOUND

                    case DSL: {
                        string dslname = name + ".dsl3d";
                        reader = new DSLConverter(dslname);
                    } break;
                }
                if (maxSideSize) {
                    reader->setMaxSideSize(maxSideSize);
                }
                if (minSides) {
                    reader->setMinSides(minSides);
                }
                reader->setUsePrimitives(options[PRIMITIVES].count() > 0);
                reader->setSplit(options[SPLIT].count() > 0);
                cout << "\nConverting files to " << formatnames[format] << "...\n";
                RVMParser parser(reader);
                if (options[OBJECT].count() > 0) {
                    parser.setObjectName(options[OBJECT].arg);
                }
                if (forcedColor != -1) {
                    parser.setForcedColor(forcedColor);
                }
                if (scale != 1) {
                    parser.setScale(scale);
                }
                vector<string> files;
                for (int file = 0; file < parse.nonOptionsCount(); file++) {
                    string filename = parse.nonOption(file);
                    files.push_back(filename);
                }
                bool res = parser.readFiles(files, name, options[SKIPATT].count() > 0);
                delete reader;
                if (!res) {
                    cout << "Conversion failed:" << endl;
                    cout << "  " << parser.lastError() << endl;
                    return 1;
                } else {
                    cout << "Statistics:" << endl;
                    cout << "  " << parser.nbGroups() << " group(s)" << endl;
                    cout << "  " << parser.nbPyramids() << " pyramid(s)" << endl;
                    cout << "  " << parser.nbBoxes() << " box(es)" << endl;
                    cout << "  " << parser.nbRectangularToruses() << " rectangular torus(es)" << endl;
                    cout << "  " << parser.nbCircularToruses() << " circular torus(es)" << endl;
                    cout << "  " << parser.nbEllipticalDishes() << " elliptical dish(es)" << endl;
                    cout << "  " << parser.nbSphericalDishes() << " spherical dish(es)" << endl;
                    cout << "  " << parser.nbSnouts() << " snout(s)" << endl;
                    cout << "  " << parser.nbCylinders() << " cylinder(s)" << endl;
                    cout << "  " << parser.nbSpheres() << " sphere(s)" << endl;
                    cout << "  " << parser.nbLines() << " line(s)" << endl;
                    cout << "  " << parser.nbFacetGroups() << " facet group(s)" << endl;
                    cout << "  " << parser.nbAttributes() << " attribute(s)" << endl;
                    time_t duration = time(0) - start;
                    cout << "Conversion done in " << (duration) << " second" << (duration > 1 ? "s" : "") << "." << endl;
                }
            }
        }
    } else {
        for (int file = 0; file < parse.nonOptionsCount(); file++) {
            string filename = parse.nonOption(file);
            for (int format = TEST + 1; format <= DUMMY; format++) {
                if (options[format].count() > 0) {
                    time_t start = time(0);
                    RVMReader* reader;
                    switch (format) {
                        case DUMMY: {
                            reader = new DummyReader;
                        } break;

#ifdef XIOT_FOUND
                        case X3D:
                        case X3DB: {
                            string name = !objectName.empty() ? objectName : filename;
                            if (options[SPLIT].count() > 0) name += "_origin";
                            name = name.substr(0, name.rfind(".")) + ".x3d" + (format == X3D ? "" : "b");
                            name = name.substr(name.rfind(PATHSEP) + 1);
                            reader = new X3DConverter(name, format == X3DB);
                        } break;
#endif // XIOT_FOUND

#ifdef OPENCOLLADASW_FOUND
                        case COLLADA: {
                            string name = !objectName.empty() ? objectName : filename;
                            name = name.substr(0, name.rfind(".")) + ".dae";
                            name = name.substr(name.rfind(PATHSEP) + 1);
                            reader = new COLLADAConverter(name);
                        } break;
#endif // OPENCOLLADASW_FOUND

                        case DSL: {
                            string name = !objectName.empty() ? objectName : filename;
                            name = name.substr(0, name.rfind(".")) + ".dsl3d";
                            name = name.substr(name.rfind(PATHSEP) + 1);
                            reader = new DSLConverter(name);
                        } break;
                    }
                    if (maxSideSize) {
                        reader->setMaxSideSize(maxSideSize);
                    }
                    if (minSides) {
                        reader->setMinSides(minSides);
                    }
                    reader->setUsePrimitives(options[PRIMITIVES].count() > 0);
                    reader->setSplit(options[SPLIT].count() > 0);
                    cout << "\nConverting file " << filename << " to " << formatnames[format] << "...\n";
                    RVMParser parser(reader);
                    if (options[OBJECT].count() > 0) {
                        parser.setObjectName(options[OBJECT].arg);
                    }
                    if (forcedColor != -1) {
                        parser.setForcedColor(forcedColor);
                    }
                    bool res = parser.readFile(filename, options[SKIPATT].count() > 0);
                    delete reader;
                    if (!res) {
                        cout << "Conversion failed:" << endl;
                        cout << "  " << parser.lastError() << endl;
                        return 1;
                    } else {
                        cout << "Statistics:" << endl;
                        cout << "  " << parser.nbGroups() << " group(s)" << endl;
                        cout << "  " << parser.nbPyramids() << " pyramid(s)" << endl;
                        cout << "  " << parser.nbBoxes() << " box(es)" << endl;
                        cout << "  " << parser.nbRectangularToruses() << " rectangular torus(es)" << endl;
                        cout << "  " << parser.nbCircularToruses() << " circular torus(es)" << endl;
                        cout << "  " << parser.nbEllipticalDishes() << " elliptical dish(es)" << endl;
                        cout << "  " << parser.nbSphericalDishes() << " spherical dish(es)" << endl;
                        cout << "  " << parser.nbSnouts() << " snout(s)" << endl;
                        cout << "  " << parser.nbCylinders() << " cylinder(s)" << endl;
                        cout << "  " << parser.nbSpheres() << " sphere(s)" << endl;
                        cout << "  " << parser.nbLines() << " line(s)" << endl;
                        cout << "  " << parser.nbFacetGroups() << " facet group(s)" << endl;
                        cout << "  " << parser.nbAttributes() << " attribute(s)" << endl;
                        time_t duration = time(0) - start;
                        cout << "Conversion done in " << (duration) << " second" << (duration > 1 ? "s" : "") << "." << endl;
                    }
                }
            }
        }
    }

    return 0;
}

