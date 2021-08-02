


#if(STX_BUILD_BENCHMARKS)
#
#if(NOT EXISTS third_party/benchmark)
#  execute_process(
#    COMMAND git submodule update --init --recursive third_party/benchmark
#    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR})
#endif()

#set(BENCHMARK_ENABLE_TESTING OFF)
#add_subdirectory(third_party/benchmark)
#endif()


#if(STX_BUILD_BENCHMARKS)
#
#  macro(add_benchmark benchmark_name file)
#    add_executable(stx_benchmark_${benchmark_name} "benchmarks/${file}")
#    target_link_libraries(stx_benchmark_${benchmark_name} stx benchmark
#                          benchmark_main)
#    target_compile_options(stx_benchmark_${benchmark_name}
#                           PRIVATE ${STX_FLAGS} ${STX_WARNING_FLAGS})
#  endmacro(add_benchmark benchmark_name)
#
#  add_benchmark(one_op one_op.cc)
#  add_benchmark(two_op two_op.cc)
#
#endif()


