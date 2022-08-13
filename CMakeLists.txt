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

execute_process(
  COMMAND git clone https://github.com/google/googletest.git -b v1.12.0
          third_party/googletest WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR})
execute_process(
  COMMAND git clone https://github.com/abseil/abseil-cpp.git -b20220623.0
          third_party/abseil WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR})

add_subdirectory(third_party/googletest)
add_subdirectory(third_party/abseil)

# ===============================================
#
# === Configuration Options
#
# ===============================================
option(STX_BUILD_TESTS "Build tests" OFF)
option(STX_SANITIZE_BUILD "Sanitize build" OFF)
option(STX_BUILD_DOCS "Build documentation" OFF)
option(
  STX_CUSTOM_PANIC_HANDLER
  "Override the default panic behaviour by implementing a custom panic handler. The default behavior is to print the panic report and abort the program. (You can read the docs for more details)"
  OFF)
option(STX_ENABLE_PANIC_BACKTRACE "Enable panic backtrace")

# ===============================================
#
# === Configuration Options Logging
#
# ===============================================
message(STATUS "[STX] Build tests: " ${STX_BUILD_TESTS})
message(STATUS "[STX] Sanitize build: " ${STX_SANITIZE_BUILD}) # not working yet
message(STATUS "[STX] Build documentation: " ${STX_BUILD_DOCS}) # not working yet
message(STATUS "[STX] Override panic handler: " ${STX_CUSTOM_PANIC_HANDLER})
message(STATUS "[STX] Enable panic backtrace: " ${STX_ENABLE_PANIC_BACKTRACE}) # not working yet

# ===============================================
#
# === Configure Flags
#
# ===============================================
if(${CMAKE_CXX_COMPILER_ID} STREQUAL "MSVC")
  add_compile_options("/W4")
else()
  if(${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU" OR ${CMAKE_CXX_COMPILER_ID}
                                                STREQUAL "Clang")
    add_compile_options("-Wpedantic" "-Wall" "-Wextra")

    if(${STX_SANITIZE_BUILD})
      add_compile_options("")
    endif()

  endif()
endif()

# ===============================================
#
# === Configuration Definitions and Platform Support
#
# ===============================================

# some microcontroller platforms 'arm-gcc-none especially" may not have
# std::thread enabled if so, we remove the thread facilities, this will cause
# compilation errors otherwise, since the classes won't be available
check_cxx_source_compiles(
  "
#include <thread>
#include <mutex>

int main(){
  std::this_thread::get_id();
  std::mutex mutex;
}
"
  LIBSTX_HAS_STD_THREAD_MUTEX)

# TODO(lamarrr): fix up
if(NOT LIBSTX_HAS_STD_THREAD_MUTEX)
  list(APPEND STX_COMPILER_DEFS "STX_NO_STD_THREAD_MUTEX")
endif()

if(${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
  add_compile_options("-fdiagnostics-color=always")
elseif(${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
  add_compile_options("-fcolor-diagnostics")
endif()

add_subdirectory(allocator)
add_subdirectory(async)
add_subdirectory(backtrace)
add_subdirectory(config)
add_subdirectory(enum)
add_subdirectory(fn)
add_subdirectory(limits)
add_subdirectory(mem)
add_subdirectory(option_result)
add_subdirectory(panic)
add_subdirectory(resource)
add_subdirectory(scheduler)
add_subdirectory(source_location)
add_subdirectory(span)
add_subdirectory(spinlock)
add_subdirectory("string")
add_subdirectory(struct)
add_subdirectory(task)
add_subdirectory(utils)
add_subdirectory(vec)
add_subdirectory(void)

# if(ANDROID) target_link_libraries(stx ${LibAtomic}) endif()
