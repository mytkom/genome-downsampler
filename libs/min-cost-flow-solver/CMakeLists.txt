add_library(
    min-cost-flow-solver STATIC
    src/push_relabel_simple_heuristics.cpp
    src/push_relabel_vector_simple_heuristics.cpp
    src/push_relabel_openmp.cpp
    src/push_relabel.cpp)

include_directories("${PROJECT_BINARY_DIR}/config")

target_include_directories(min-cost-flow-solver PUBLIC ${CMAKE_SOURCE_DIR}/libs/min-cost-flow-solver/include)
target_link_libraries(min-cost-flow-solver PRIVATE logging  OpenMP::OpenMP_CXX)
target_compile_options(${PROJECT_NAME} PRIVATE ${PROJ_CXX_FLAGS})
