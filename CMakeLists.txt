cmake_minimum_required(VERSION 3.13)

project(
  Stx
  VERSION 0.0.1
  DESCRIPTION "A Set of Monadic Extensions to the C++ Standard Library"
  LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED 20)

option(STX_BUILD_TESTS "Build tests" OFF)
option(STX_BUILD_DOCS "Build Documentation" OFF)
option(STX_BUILD_BENCHMARKS "Build benchmarks" OFF)
option(STX_SANITIZE_TESTS "Build sanitized tests" OFF)
option(STX_OVERRIDE_PANIC_HANDLER
       "Override the default panic behaviour (See: )" OFF)
option(STX_BUILD_DOCS "Build Documentation" OFF)

list(APPEND STX_COMPILER_DEFS)
if(STX_OVERRIDE_PANIC_HANDLER)
  list(APPEND STX_COMPILER_DEFS "STX_OVERRIDE_PANIC_HANDLER")
endif()

list(APPEND STX_WARNING_FLAGS)
if(${CMAKE_CXX_COMPILER_ID} MATCHES "Clang|GNU")
  list(APPEND STX_WARNING_FLAGS "-Wpedantic" "-Wall" "-Wextra")
elseif(MSVC)
  list(APPEND STX_WARNING_FLAGS "/W3" "/WX")
endif()

include(CheckIncludeFileCXX)
check_include_file_cxx(experimental/source_location
                       EXPERIMENTAL_SOURCE_LOCATION)
check_include_file_cxx(source_location STABLE_SOURCE_LOCATION)

if(EXPERIMENTAL_SOURCE_LOCATION OR STABLE_SOURCE_LOCATION)
  if(STABLE_SOURCE_LOCATION)
    list(APPEND STX_COMPILER_DEFS "STX_STABLE_SOURCE_LOCATION")
  endif()
else()
  message(
    FATAL_ERROR
      "The C++ 20 source location library is not available on this toolchain")
endif()

list(APPEND STX_SRCS src/stx.cc src/panic/panic.cc
            src/panic/handlers/throw/panic_info.cc)

add_library(stx ${STX_SRCS})
target_include_directories(stx PUBLIC include)
set_property(TARGET stx PROPERTY CXX_STANDARD 20)
target_compile_options(stx PRIVATE ${STX_WARNING_FLAGS})
target_compile_definitions(stx PUBLIC ${STX_COMPILER_DEFS})

if(STX_BUILD_TESTS OR STX_BUILD_BENCHMARKS)
  set(FMT_TEST OFF)
  add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/third_party/fmt)
endif()

list(APPEND STX_TEST_SRCS tests/tests.cc tests/panic_test.cc
            tests/result_test.cc tests/common_test.cc tests/option_test.cc)

add_executable(example_divide examples/divide.cc)
target_link_libraries(example_divide stx)
set_property(TARGET example_divide PROPERTY CXX_STANDARD 20)

add_executable(example_parse examples/parse_version.cc)
target_link_libraries(example_parse stx)
set_property(TARGET example_parse PROPERTY CXX_STANDARD 20)

if(STX_BUILD_TESTS)

  set(BUILD_GMOCK OFF)
  add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/third_party/googletest)

  add_executable(stx_tests ${STX_TEST_SRCS})
  target_link_libraries(stx_tests stx ${GTEST} ${PTHREAD} gtest_main gtest)
  target_compile_options(stx_tests PRIVATE ${STX_WARNING_FLAGS})
  set_property(TARGET stx_tests PROPERTY CXX_STANDARD 20)
  add_custom_target(
    run_tests
    ${CMAKE_CURRENT_BINARY_DIR}/stx_tests
    DEPENDS stx_tests)

  macro(add_sanitized_test sanitize_mode)
    add_executable("stx_tests_${sanitize_mode}_sanitized" ${STX_TEST_SRCS})
    target_compile_options(
      "stx_tests_${sanitize_mode}_sanitized"
      PRIVATE ${STX_WARNING_FLAGS} "-fsanitize=${sanitize_mode}")
    target_link_libraries("stx_tests_${sanitize_mode}_sanitized" stx gtest_main
                          gtest "-fsanitize=${sanitize_mode}")
    set_property(TARGET "stx_tests_${sanitize_mode}_sanitized"
                 PROPERTY CXX_STANDARD 20)
  endmacro(add_sanitized_test sanitize_mode)

  if(STX_SANITIZE_TESTS)
    add_sanitized_test(undefined)
    add_sanitized_test(address)
    add_sanitized_test(leak)
    add_sanitized_test(thread)
  else()

  endif()
endif()

if(STX_BUILD_BENCHMARKS)

  set(BENCHMARK_ENABLE_TESTING OFF)
  add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/third_party/benchmark)

  add_executable(compare_bench benchmarks/compare.cc)
  target_link_libraries(compare_bench stx fmt::fmt)
  set_property(TARGET compare_bench PROPERTY CXX_STANDARD 20)
  target_compile_options(compare_bench PRIVATE ${STX_WARNING_FLAGS})

  macro(add_benchmark benchmark_name file)
    add_executable("stx_benchmark_${benchmark_name}" "benchmarks/${file}")
    target_link_libraries("stx_benchmark_${benchmark_name}" stx benchmark
                          benchmark_main)
    set_property(TARGET "stx_benchmark_${benchmark_name}" PROPERTY CXX_STANDARD
                                                                   20)
    target_compile_options("stx_benchmark_${benchmark_name}"
                           PRIVATE ${STX_WARNING_FLAGS})
  endmacro(add_benchmark benchmark_name)

  add_benchmark(one_op one_op.cc)
  add_benchmark(two_op two_op.cc)

endif()

if(STX_BUILD_DOCS)
  find_package(Doxygen REQUIRED)
  set(DOXYGEN_IN ${CMAKE_CURRENT_LIST_DIR}/Doxyfile.in)
  set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/docs/Doxyfile)

  # request to configure the file
  configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
  message("Doxygen build started")

  # note the option ALL which allows to build the docs together with the
  # application
  add_custom_target(
    docs ALL
    COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
    COMMENT "Generating API documentation with Doxygen"
    VERBATIM)

endif()
