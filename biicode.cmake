find_package(OpenGL REQUIRED)

ADD_BII_TARGETS()
TARGET_LINK_LIBRARIES(${BII_BLOCK_TARGET} INTERFACE ${OPENGL_LIBRARIES})

ADD_DEFINITIONS(-DXIOT_FOUND -DOPENCOLLADASW_FOUND)

# Rename src/main.cpp to pmuc
SET_TARGET_PROPERTIES(${BII_src_main_TARGET} PROPERTIES OUTPUT_NAME pmuc)

add_test(NAME run_pmuc COMMAND ${BII_src_main_TARGET} --help)
set_tests_properties(run_pmuc PROPERTIES PASS_REGULAR_EXPRESSION "usage")
set_tests_properties(run_pmuc PROPERTIES PASS_REGULAR_EXPRESSION "--x3d")
set_tests_properties(run_pmuc PROPERTIES PASS_REGULAR_EXPRESSION "--collada")

add_test(NAME pmuc_collada COMMAND ${BII_src_main_TARGET} --collada ${CMAKE_CURRENT_SOURCE_DIR}/data/plm-sample_11072013.rvm)
add_test(NAME pmuc_x3d COMMAND ${BII_src_main_TARGET} --x3d ${CMAKE_CURRENT_SOURCE_DIR}/data/plm-sample_11072013.rvm)
add_test(NAME pmuc_x3db COMMAND ${BII_src_main_TARGET} --x3db ${CMAKE_CURRENT_SOURCE_DIR}/data/plm-sample_11072013.rvm)


set_tests_properties(pmuc_collada pmuc_x3d pmuc_x3db PROPERTIES PASS_REGULAR_EXPRESSION "315 group")
set_tests_properties(pmuc_collada pmuc_x3d pmuc_x3db PROPERTIES PASS_REGULAR_EXPRESSION "118 pyramid")
set_tests_properties(pmuc_collada pmuc_x3d pmuc_x3db PROPERTIES PASS_REGULAR_EXPRESSION "173 box")
set_tests_properties(pmuc_collada pmuc_x3d pmuc_x3db PROPERTIES PASS_REGULAR_EXPRESSION "74 rectangualr torus")
set_tests_properties(pmuc_collada pmuc_x3d pmuc_x3db PROPERTIES PASS_REGULAR_EXPRESSION "38 circular torus")
set_tests_properties(pmuc_collada pmuc_x3d pmuc_x3db PROPERTIES PASS_REGULAR_EXPRESSION "  2 elliptical dish")
set_tests_properties(pmuc_collada pmuc_x3d pmuc_x3db PROPERTIES PASS_REGULAR_EXPRESSION "  0 spherical dish")
set_tests_properties(pmuc_collada pmuc_x3d pmuc_x3db PROPERTIES PASS_REGULAR_EXPRESSION "  22 snout")
set_tests_properties(pmuc_collada pmuc_x3d pmuc_x3db PROPERTIES PASS_REGULAR_EXPRESSION "  139 cylinder")
set_tests_properties(pmuc_collada pmuc_x3d pmuc_x3db PROPERTIES PASS_REGULAR_EXPRESSION "  0 sphere")
set_tests_properties(pmuc_collada pmuc_x3d pmuc_x3db PROPERTIES PASS_REGULAR_EXPRESSION "  12 line")
set_tests_properties(pmuc_collada pmuc_x3d pmuc_x3db PROPERTIES PASS_REGULAR_EXPRESSION "  80 facet group")

