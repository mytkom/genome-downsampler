add_library(gds_c SHARED src/gds.cpp)

target_compile_definitions(gds_c PRIVATE GDS_BUILD_SHARED)

target_include_directories(gds_c
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
  PRIVATE ${CMAKE_SOURCE_DIR}/src ${PROJECT_BINARY_DIR}/config)

target_link_libraries(gds_c PRIVATE bam-api qmcp-solver logging min-cost-flow-solver
                                    OpenMP::OpenMP_CXX)

target_compile_options(gds_c PRIVATE ${PROJ_CXX_FLAGS})

set_target_properties(gds_c PROPERTIES CXX_VISIBILITY_PRESET hidden VISIBILITY_INLINES_HIDDEN ON
                                       POSITION_INDEPENDENT_CODE ON)

install(TARGETS gds_c LIBRARY DESTINATION lib)
install(FILES include/gds.h DESTINATION include)
