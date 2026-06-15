add_executable(solver-bench bench.cpp)

target_link_libraries(solver-bench PRIVATE
  benchmark::benchmark_main
  qmcp-solver min-cost-flow-solver reads-gen bam-api logging)
target_compile_options(solver-bench PRIVATE ${PROJ_CXX_FLAGS})

