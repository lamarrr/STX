# MIT License
#
# Copyright (c) 2020-2022 Basit Ayantunde
#

cmake_minimum_required(VERSION 3.13)

project(
  Stx
  VERSION 1.0.0
  DESCRIPTION "A set of utility extensions to the C++ Standard Library"
  HOMEPAGE_URL "https://github.com/lamarrr/STX"
  LANGUAGES CXX)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake-modules)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(CheckCXXSourceCompiles)
include(add_project_library)

# ===============================================
#
# === Configuration Options
#
# ===============================================
option(STX_BUILD_TESTS "Build tests" OFF)
option(STX_BUILD_DOCS "Build documentation" OFF)
option(
  STX_CUSTOM_PANIC_HANDLER
  "Override the default panic behaviour by implementing a custom panic handler. The default behavior is to print the panic report and abort the program. (You can read the docs for more details)"
  OFF)
option(STX_ENABLE_BACKTRACE "Enable stack backtraces" OFF)
# ===============================================
#
# === Configuration Options Logging
#
# ===============================================
message(STATUS "[STX] Build tests: " ${STX_BUILD_TESTS})
message(STATUS "[STX] Build documentation: " ${STX_BUILD_DOCS}) # not working
                                                                # yet
message(STATUS "[STX] Override panic handler: " ${STX_CUSTOM_PANIC_HANDLER})

# ===============================================
#
# === Dependencies
#
# ===============================================
find_package(Threads REQUIRED)

if(${STX_BUILD_TESTS})
  find_package(GTest CONFIG REQUIRED)
endif()

if(${STX_ENABLE_BACKTRACE})
  find_package(absl CONFIG REQUIRED)
endif()

# ===============================================
#
# === Configuration Definitions and Platform Support
#
# ===============================================

# some microcontroller platforms 'arm-gcc-none especially" may not have
# std::thread enabled if so, we remove the thread facilities, this will cause
# compilation errors otherwise, since the classes won't be available

if(UNIX)
  add_link_options("-latomic")
endif()

check_cxx_source_compiles(
  "
  #include <thread>

  int main(){
    std::this_thread::get_id();
  }
"
  LIBSTX_HAS_STD_THREAD)

if(NOT LIBSTX_HAS_STD_THREAD)
  list(APPEND STX_COMPILER_DEFS "STX_NO_STD_THREAD")
endif()

file(GLOB STX_SOURCE_FILES_LIST src/*.cc)

add_library(stx STATIC ${STX_SOURCE_FILES_LIST})
target_link_libraries(stx Threads::Threads)

add_library(stx::stx ALIAS stx)

if(${STX_ENABLE_BACKTRACE})
  target_link_libraries(stx absl::stacktrace absl::symbolize)
  target_compile_definitions(stx PRIVATE "STX_ENABLE_BACKTRACE")
endif()

set_target_properties(stx PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON)

target_include_directories(stx PUBLIC include)
target_include_directories(stx PRIVATE src)

if(MSVC)
  target_compile_options(stx PUBLIC "/permissive-")
endif()

# ===============================================
#
# === Install
#
# ===============================================
install(
  TARGETS stx
  EXPORT stx
  LIBRARY DESTINATION lib)
install(DIRECTORY include/stx DESTINATION include)

# ===============================================
#
# === Tests
#
# ===============================================

file(GLOB STX_TESTS_SOURCE_FILES_LIST tests/*.cc)

if(${STX_BUILD_TESTS})
  if(STX_TESTS_SOURCE_FILES_LIST)
    add_executable(stx_tests ${STX_TESTS_SOURCE_FILES_LIST})
    target_include_directories(stx_tests PRIVATE tests)
    target_link_libraries(stx_tests GTest::gtest GTest::gtest_main stx::stx)
  endif()
endif()
