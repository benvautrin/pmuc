/*
 * Plant Mock-Up Converter
 *
 * Copyright (c) 2016, EDF. All rights reserved.
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
#include "api/rvmprimitive.h"
#include "converters/ifcconverter.h"
#include "converters/x3dconverter.h"
#include "converters/colladaconverter.h"

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

enum optionIndex { UNKNOWN, HELP, TEST,  X3D,
    X3DB, COLLADA, IFC4, IFC2X3, DSL, DUMMY,
    SKIPATT, SPLIT, AGGREGATE, PRIMITIVES, SIDESIZE,
    MINSIDES, OBJECT, COLOR, SCALE };

const option::Descriptor usage[] = {
    { UNKNOWN,      0, "",  "",              option::Arg::None,      "\nusage: pmuc [options] <rvm file 1> ...\n\nChoose at least one format and one file to convert.\nOptions:" },
    { HELP,         0, "h", "help",          option::Arg::None,      "  --help, -h \tPrint usage and exit." },
    { X3D,          0, "",  "x3d",           option::Arg::None,      "  --x3d  \tConvert to X3D XML format." },
    { X3DB,         0, "",  "x3db",          option::Arg::None,      "  --x3db  \tConvert to X3D binary format." },
    { COLLADA,      0, "",  "collada",       option::Arg::None,      "  --collada\tConvert to COLLADA format." },
    { IFC2X3,       0, "",  "ifc",           option::Arg::None,      "  --ifc\tConvert to IFC2x3." },
    { IFC4,         0, "",  "ifc4",          option::Arg::None,      "  --ifc4\tConvert to IFC4." },
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
    "", "", "",
    "X3D", "X3DB",
    "COLLADA",
    "IFC4", "IFC2x3",
    "DSL", "DUMMY",
};

enum primitives { BOX, SNOUT, CYLINDER, SPHERE, CIRCULARTORUS, RECTANGULARTORUS, PYRAMID, LINE, ELLIPTICALDISH, SPHERICALDISH };
const string primitiveNames[] = { "box", "snout", "cylinder", "sphere", "circulartorus", "rectangulartorus", "pyramid", "line", "ellipticaldish", "sphericaldish" };

void printStats(time_t duration, RVMParser &parser) {
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

    cout << "Conversion done in " << (duration) << " second" << (duration > 1 ? "s" : "") << "." << endl;
}

int main(int argc, char** argv)
{
    cout << "Plant Mock-Up Converter 1.1.1\nCopyright (C) EDF 2017" << endl;

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

    if ((options[X3D] || options[X3DB] || options[COLLADA] || options[DSL] || options[DUMMY] || options[IFC4]|| options[IFC2X3]) == 0) {
        cerr << "\nNo format specified.\n";
        option::printUsage(std::cerr, usage);
        return 1;
    }

    if (parse.nonOptionsCount() == 0 && options[TEST].count() == 0) {
        cerr << "\nNo file specified.\n";
        option::printUsage(std::cerr, usage);
        return 1;
    }

    int minSides = 16;
    if (options[MINSIDES].count() > 0) {
        minSides = atoi(options[MINSIDES].arg);
        if (minSides < 5) {
            cerr << "\n--minsides option should be > 4.\n";
            option::printUsage(std::cerr, usage);
            return 1;
        }
    }

    float maxSideSize = 25.;
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
                        case X3D:
                        case X3DB: {
                            string name = filename + ".x3d" + (format == X3D ? "" : "b");
                            reader = new X3DConverter(name, format == X3DB);
                        } break;

#ifdef OPENCOLLADASW_FOUND
                        case COLLADA: {
                            string name = filename + ".dae";
                            reader = new COLLADAConverter(name);
                        } break;
#endif // OPENCOLLADASW_FOUND

                        case IFC4: {
                            string name = filename + ".ifc";
                            reader = new IFCConverter(name, "IFC4");
                        } break;

                        case IFC2X3: {
                            string name = filename + ".ifc";
                            reader = new IFCConverter(name, "IFC2X3");
                        } break;


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
                    std::array<float, 12> matrix;
                    for (int j = 0; j < 12; j++) matrix[j] = 0.0f;
                    matrix[0] = matrix[4] = matrix[8] = 1.0f;

                    reader->startDocument();
                    reader->startHeader("Plant Mock-Up Converter", "Primitive example file", "", "", "");
                    reader->endHeader();
                    reader->startModel("Primitive examples", primitiveNames[i]);
                    reader->startGroup(primitiveNames[i], translation, forcedColor != -1 ? forcedColor : 2);
                    switch (i) { // BOX, SNOUT, CYLINDER, SPHERE, CIRCULARTORUS, RECTANGULARTORUS, PYRAMID, LINE, ELLIPTICALDISH, SPHERICALDISH
                        case BOX: {
                            Primitives::Box  box;
                            box.len[0] = 1.0;
                            box.len[1] = 1.0;
                            box.len[2] = 1.0;
                            reader->createBox(matrix, box);
                        } break;
                        case SNOUT: {
                            Primitives::Snout snout;
                            snout.data[0] = 2.0f; // bottom radius
                            snout.data[1] = 2.0f; // top radius
                            snout.data[2] = 5.0f;  // height
                            snout.data[3] = 0.0f;  // xoffset
                            snout.data[4] = 0.0f;  // yoffset
                            snout.data[5] = 0.0f; // xbottomNormalOffset
                            snout.data[6] = 0.4f;  // ybottomNormalOffset
                            snout.data[7] = 0.0f;  // xtopNormalOffset
                            snout.data[8] = -0.4f; // ytopNormalOffset

                            reader->createSnout(matrix, snout);
                        } break;
                        case CYLINDER: {
                            Primitives::Cylinder  cylinder;
                            cylinder.data[0] = 1.0f;
                            cylinder.data[1] = 2.0f;
                            reader->createCylinder(matrix, cylinder);
                        } break;
                        case SPHERE: {
                            Primitives::Sphere sphere;
                            sphere.diameter = 2.0;
                            reader->createSphere(matrix, sphere);
                        } break;
                        case CIRCULARTORUS: {
                            Primitives::CircularTorus torus;
                            torus.data[0] = 4;
                            torus.data[1] = 2;
                            torus.data[2] = (float)M_PI;
                            reader->createCircularTorus(matrix, torus);
                        } break;
                        case RECTANGULARTORUS: {
                            Primitives::RectangularTorus torus;
                            torus.data[0] = 7.5;  // inside
                            torus.data[1] = 8.0;  // outside
                            torus.data[2] = 2.0;  // height
                            torus.data[3] = (float)M_PI*1.5f;
                            reader->createRectangularTorus(matrix, torus);
                        } break;
                        case PYRAMID: {
                            Primitives::Pyramid pyramid;
                            pyramid.data[0] = 2; // xbottom
                            pyramid.data[1] = 4; // ybottom
                            pyramid.data[2] = 4; // xtop
                            pyramid.data[3] = 4; // ytop
                            pyramid.data[4] = 0; // xoffset
                            pyramid.data[5] = 0; // yoffset
                            pyramid.data[6] = 4; // height

                            reader->createPyramid(matrix, pyramid);
                        } break;
                        case LINE: {
                            reader->createLine(matrix, 1, 2);
                        } break;
                        case ELLIPTICALDISH: {
                            Primitives::EllipticalDish dish;
                            dish.data[0] = 4;
                            dish.data[1] = 2;
                            reader->createEllipticalDish(matrix, dish);
                        } break;
                        case SPHERICALDISH: {
                            Primitives::SphericalDish dish;
                            dish.data[0] = 4;
                            dish.data[1] = 1;
                            reader->createSphericalDish(matrix, dish);
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

                    case X3D:
                    case X3DB: {
                        string x3dname = name + ".x3d" + (format == X3D ? "" : "b");
                        reader = new X3DConverter(x3dname, format == X3DB);
                    } break;

                    case COLLADA: {
                        string colladaname = name + ".dae";
                        reader = new COLLADAConverter(colladaname);
                    } break;

                    case DSL: {
                        string dslname = name + ".dsl3d";
                        reader = new DSLConverter(dslname);
                    } break;

                    case IFC2X3: {
                        string ifcname = name + ".ifc";
                        reader = new IFCConverter(ifcname, "IFC4");
                    } break;
                    case IFC4: {
                        string ifcname = name + ".ifc";
                        reader = new IFCConverter(ifcname, "IFC2X3");
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
                RVMParser parser(*reader);
                if (options[OBJECT].count() > 0) {
                    parser.setObjectName(options[OBJECT].arg);
                }
                if (forcedColor != -1) {
                    parser.setForcedColor(forcedColor);
                }
                parser.setScale(scale);

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
                    printStats(time(0) - start, parser);
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

                        case X3D:
                        case X3DB: {
                            string name = !objectName.empty() ? objectName : filename;
                            if (options[SPLIT].count() > 0) name += "_origin";
                            name = name.substr(0, name.rfind(".")) + ".x3d" + (format == X3D ? "" : "b");
                            name = name.substr(name.rfind(PATHSEP) + 1);
                            reader = new X3DConverter(name, format == X3DB);
                        } break;

                        case COLLADA: {
                            string name = !objectName.empty() ? objectName : filename;
                            name = name.substr(0, name.rfind(".")) + ".dae";
                            name = name.substr(name.rfind(PATHSEP) + 1);
                            reader = new COLLADAConverter(name);
                        } break;

                        case IFC4: {
                            string name = !objectName.empty() ? objectName : filename;
                            name = name.substr(0, name.rfind(".")) + ".ifc";
                            name = name.substr(name.rfind(PATHSEP) + 1);
                            reader = new IFCConverter(name, "IFC4");
                        } break;
                        case IFC2X3: {
                            string name = !objectName.empty() ? objectName : filename;
                            name = name.substr(0, name.rfind(".")) + ".ifc";
                            name = name.substr(name.rfind(PATHSEP) + 1);
                            reader = new IFCConverter(name, "IFC2X3");
                        } break;

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
                    RVMParser parser(*reader);
                    if (options[OBJECT].count() > 0) {
                        parser.setObjectName(options[OBJECT].arg);
                    }
                    if (forcedColor != -1) {
                        parser.setForcedColor(forcedColor);
                    }
                    parser.setScale(scale);

                    bool res = parser.readFile(filename, options[SKIPATT].count() > 0);
                    delete reader;
                    if (!res) {
                        cout << "Conversion failed:" << endl;
                        cout << "  " << parser.lastError() << endl;
                        return 1;
                    } else {
                        printStats(time(0) - start, parser);
                    }
                }
            }
        }
    }

    return 0;
}
