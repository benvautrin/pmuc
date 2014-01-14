project(pmuc)
cmake_minimum_required(VERSION 2.8)

SET(CMAKE_MODULE_PATH "${pmuc_SOURCE_DIR}/CMakeModules;${CMAKE_MODULE_PATH}")

find_package(XIOT REQUIRED)
include_directories( ${XIOT_INCLUDE_DIR} )
find_package(OpenCOLLADASW REQUIRED)
include_directories( ${OPENCOLLADASW_INCLUDE_DIR} )
include_directories( ${OPENCOLLADABU_INCLUDE_DIR} )
find_package(EIGEN REQUIRED)
include_directories( ${EIGEN_INCLUDE_DIR} )
find_package(Iconv)
include_directories( ${ICONV_INCLUDE_DIR} )

if (ICONV_FOUND)
    add_definitions( -DICONV_FOUND )
endif (ICONV_FOUND)

aux_source_directory(src SRC_LIST)
aux_source_directory(src/api APISRC_LIST)
aux_source_directory(src/converters CONVERTERSSRC_LIST)

add_library(rvm ${APISRC_LIST})
target_link_libraries(rvm STATIC ${ICONV_LIBRARY})

add_executable(${PROJECT_NAME} ${SRC_LIST} ${APISRC_LIST} ${CONVERTERSSRC_LIST})
target_link_libraries(${PROJECT_NAME} ${OPENCOLLADASW_LIBRARIES})
target_link_libraries(${PROJECT_NAME} ${XIOT_LIBRARY})
target_link_libraries(${PROJECT_NAME} ${ICONV_LIBRARY})
