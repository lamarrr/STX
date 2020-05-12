cmake_minimum_required(VERSION 3.13)

project(
  Stx
  VERSION 0.0.1
  DESCRIPTION "A Set of Extensions to the C++ Standard Library"
  LANGUAGES CXX)

include(CMakeDependentOption)
include(CheckIncludeFileCXX)

set(CMAKE_CXX_STANDARD_REQUIRED 20)

option(STX_BUILD_TESTS "Build tests" OFF)
option(STX_BUILD_DOCS "Build documentation" OFF)
option(STX_BUILD_BENCHMARKS "Build benchmarks" OFF)
cmake_dependent_option(STX_SANITIZE_TESTS "Build sanitized tests" ON
                       "STX_BUILD_TESTS" OFF)
option(STX_OVERRIDE_PANIC_HANDLER
       "Override the default panic behaviour (See: )" OFF)
option(STX_USE_LIB_CPP "Use Clang's libc++" OFF) # clang 11 doesn't have the
                                                 # source location library as of
                                                 # 9/5/2020
option(STX_ENABLE_PANIC_HOOKS "Enable runtime panic hooks" OFF)

# size for the panic description, any size longer than this will be truncated
set(STX_PANIC_REPORT_SIZE 256)

message(STATUS "[STX] Build tests: " ${STX_BUILD_TESTS})
message(STATUS "[STX] Build documentation: " ${STX_BUILD_DOCS})
message(STATUS "[STX] Build benchmarks: " ${STX_BUILD_BENCHMARKS})
message(STATUS "[STX] Sanitize tests: " ${STX_SANITIZE_TESTS})
message(STATUS "[STX] Override panic handler: " ${STX_OVERRIDE_PANIC_HANDLER})
message(STATUS "[STX] Use libc++: " ${STX_USE_STDLIB_CPP})
message(STATUS "[STX] Panic info's report size: " ${STX_PANIC_REPORT_SIZE})
message(STATUS "[STX] Use clang's libc++" ${STX_USE_LIB_CPP})

if(STX_USE_LIB_CPP) # same for MSVC?
  list(APPEND CMAKE_CXX_FLAGS -stdlib=libc++)
endif()

list(APPEND STX_COMPILER_DEFS)
if(STX_OVERRIDE_PANIC_HANDLER)
  list(APPEND STX_COMPILER_DEFS "STX_OVERRIDE_PANIC_HANDLER")
endif()

# only compilers we know of, MSVC, GNU, NVCC safer

list(APPEND STX_WARNING_FLAGS)
if(${CMAKE_CXX_COMPILER_ID} MATCHES "Clang|GNU")
  list(APPEND STX_WARNING_FLAGS "-Wpedantic" "-Wall" "-Wextra")
elseif(MSVC)
  list(APPEND STX_WARNING_FLAGS "/W3" "/WX")
endif()

list(APPEND STX_CHECK_FLAGS)
if(${CMAKE_CXX_COMPILER_ID} MATCHES "Clang|GNU")
  list(APPEND STX_CHECK_FLAGS "-std=c++20")
elseif(MSVC)
  list(APPEND STX_CHECK_FLAGS "/std:c++20")
endif()

# tests make use of macros which don't use the result, especially
# EXPECT_ANY_THROW
list(APPEND STX_TEST_WARNING_FLAGS)
if(${CMAKE_CXX_COMPILER_ID} MATCHES "Clang|GNU")
  list(APPEND STX_TEST_WARNING_FLAGS "-Wno-unused-result")
  # add MSVC flag
endif()

check_include_file_cxx(experimental/source_location
                       EXPERIMENTAL_SOURCE_LOCATION ${STX_CHECK_FLAGS})
check_include_file_cxx(source_location STABLE_SOURCE_LOCATION
                       ${STX_CHECK_FLAGS})

if(EXPERIMENTAL_SOURCE_LOCATION OR STABLE_SOURCE_LOCATION)
  if(STABLE_SOURCE_LOCATION)
    list(APPEND STX_COMPILER_DEFS "STX_STABLE_SOURCE_LOCATION")
  endif()
else()
  message(
    FATAL_ERROR
      "The C++ 20 source location library is not available on this toolchain, consider changing the CC and CXX environment variable to a newer one"
  )
endif()

list(APPEND STX_SRCS src/stx.cc src/panic.cc src/report.cc src/hook.cc)

add_library(stx ${STX_SRCS})
target_include_directories(stx PUBLIC include)
set_property(TARGET stx PROPERTY CXX_STANDARD 20)
target_compile_options(stx PRIVATE ${STX_WARNING_FLAGS})
target_compile_definitions(stx PUBLIC ${STX_COMPILER_DEFS})

if(STX_BUILD_TESTS OR STX_BUILD_BENCHMARKS)
  set(FMT_TEST OFF)
  add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/third_party/fmt)
endif()

list(
  APPEND STX_TEST_SRCS
         tests/tests.cc
         tests/panic_info.cc
         tests/panic_test.cc
         tests/result_test.cc
         tests/common_test.cc
         tests/option_test.cc
         tests/report_test.cc)

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
  target_include_directories(stx_tests PRIVATE tests)
  target_compile_options(stx_tests PRIVATE ${STX_WARNING_FLAGS}
                                           ${STX_TEST_WARNING_FLAGS})
  target_compile_definitions(stx_tests PRIVATE STX_OVERRIDE_PANIC_HANDLER)
  set_property(TARGET stx_tests PROPERTY CXX_STANDARD 20)
  add_custom_target(
    run_tests
    ${CMAKE_CURRENT_BINARY_DIR}/stx_tests
    DEPENDS stx_tests)

  macro(add_sanitized_test sanitize_mode)
    add_executable("stx_tests_${sanitize_mode}_sanitized" ${STX_TEST_SRCS})
    target_compile_options(
      "stx_tests_${sanitize_mode}_sanitized"
      PRIVATE "-fsanitize=${sanitize_mode}" ${STX_WARNING_FLAGS}
              ${STX_TEST_WARNING_FLAGS})
    target_compile_definitions("stx_tests_${sanitize_mode}_sanitized"
                               PRIVATE STX_OVERRIDE_PANIC_HANDLER)
    target_include_directories("stx_tests_${sanitize_mode}_sanitized"
                               PRIVATE tests)
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
