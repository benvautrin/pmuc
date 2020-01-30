project(PMUC)
cmake_minimum_required(VERSION 2.8)

# Set C++11 flags to makesome function work
if(UNIX)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=gnu++0x")
endif()

set (SRC_LIST
    ../src/api/rvmcolorhelper.cpp
    ../src/api/rvmparser.cpp
    ../src/api/rvmreader.cpp
    ../src/api/vector3f.cpp
)

set (HEADER_LIST
    ../src/api/lib_export.h
    ../src/api/rvmcolorhelper.h
    ../src/api/rvmparser.h
    ../src/api/rvmprimitive.h
    ../src/api/rvmreader.h
    ../src/api/vector3f.h
)

set (USE_MESH_HELPER TRUE CACHE BOOL 
     "Indicates whether it is needed to include rvmmeshhelper class in the library")

if (USE_MESH_HELPER)
  # Add OpenGL dependencies
  set (OpenGL_GL_PREFERENCE LEGACY)
  find_package (OpenGL REQUIRED)

  list (APPEND SRC_LIST ../src/api/rvmmeshhelper.cpp)
  list (APPEND HEADER_LIST ../src/api/rvmmeshhelper.h)

endif()

add_library (${PROJECT_NAME} SHARED "${SRC_LIST}")

set_target_properties (${PROJECT_NAME} PROPERTIES PUBLIC_HEADER "${HEADER_LIST}")


install(TARGETS ${PROJECT_NAME}
        CONFIGURATIONS Release
        RUNTIME DESTINATION bin
        ARCHIVE DESTINATION lib
	LIBRARY DESTINATION lib
	PUBLIC_HEADER DESTINATION include/pmuc)

install(TARGETS ${PROJECT_NAME}
	CONFIGURATIONS Debug
        RUNTIME DESTINATION bind
	ARCHIVE DESTINATION libd
	LIBRARY DESTINATION libd
        PUBLIC_HEADER DESTINATION include/pmuc)

install(TARGETS ${PROJECT_NAME}
	CONFIGURATIONS RelWithDebInfo
        RUNTIME DESTINATION bini
	ARCHIVE DESTINATION libi
	LIBRARY DESTINATION libi
        PUBLIC_HEADER DESTINATION include/pmuc)

if (WIN32)
  install(FILES $<TARGET_PDB_FILE:${PROJECT_NAME}> CONFIGURATIONS Debug
          DESTINATION bind OPTIONAL)
  install(FILES $<TARGET_PDB_FILE:${PROJECT_NAME}> CONFIGURATIONS RelWithDebInfo
          DESTINATION bini OPTIONAL)
endif()

if (USE_MESH_HELPER)
  # Include OpenGL lib
  target_link_libraries(${PROJECT_NAME} ${OPENGL_LIBRARIES})
endif()

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 11)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD_REQUIRED ON)
