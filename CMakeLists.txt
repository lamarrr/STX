# MIT License
#
# Copyright (c) 2020-2021 Basit Ayantunde
#

cmake_minimum_required(VERSION 3.13)

project(
  Stx
  VERSION 2.0.0
  DESCRIPTION "A set of utility extensions to the C++ Standard Library"
  HOMEPAGE_URL "https://github.com/lamarrr/STX"
  LANGUAGES CXX)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake-modules)

include(CMakeDependentOption)
include(CheckCXXSourceCompiles)
include(add_project_library)

find_package(GTest REQUIRED)
find_package(absl REQUIRED)

# ===============================================
#
# === Configuration Options
#
# ===============================================
option(STX_BUILD_TESTS "Build tests" OFF)
option(STX_SANITIZE_BUILD "Sanitize build" OFF)
option(STX_BUILD_DOCS "Build documentation" OFF)
option(STX_BUILD_BENCHMARKS "Build benchmarks" OFF)
option(
  STX_CUSTOM_PANIC_HANDLER
  "Override the default panic behaviour by providing a custom panic handler (You can read the docs for more details)"
  OFF)

# this feature replaces the default panic handler, by default it is set to print
# the panic report and abort the program. The handler is thread-safe.

# the default panic hook calls the provided panic handler. the behaviour can be
# changed at runtime especially when built as a shared library. Making panic
# hooks visible enables you to control the panic behaviour of the dynamically
# loaded library that depends on STX, i.e. for device drivers you might not want
# them to cause the whole program to fail on a panic.
#
# option(STX_VISIBLE_PANIC_HOOK "Make runtime panic hook visible")

# STX_PANIC_INCLUDE_BACKTRACE

# enables the backtrace library, we won't allow it to work by default when built
# as a shared library for now until we find a reason to, If you find it
# inconvenient please drop a PR!.
#
# option(STX_ENABLE_BACKTRACE "Enables the backtrace feature" OFF)

# disables printing backtraces on panic by the default panic handler. This
# requires that panic backtraces are enabled and that the panic handler is not
# overriden.
#
# cmake_dependent_option( STX_ENABLE_PANIC_BACKTRACE "Enables the panic
# backtrace feature" ON "STX_ENABLE_BACKTRACE;NOT STX_CUSTOM_PANIC_HANDLER" OFF)

# ===============================================
#
# === Configuration Options Logging
#
# ===============================================

message(STATUS "[STX] Build tests: " ${STX_BUILD_TESTS})
message(STATUS "[STX] Sanitize build: " ${STX_SANITIZE_BUILD})
message(STATUS "[STX] Build documentation: " ${STX_BUILD_DOCS})
message(STATUS "[STX] Build benchmarks: " ${STX_BUILD_BENCHMARKS})
message(STATUS "[STX] Override panic handler: " ${STX_CUSTOM_PANIC_HANDLER})

# message(STATUS "[STX] Make runtime panic hook visible: "
# ${STX_VISIBLE_PANIC_HOOK})
#
# message(STATUS "[STX] Enable panic backtrace: " ${STX_ENABLE_PANIC_BACKTRACE})
#
# message(STATUS "[STX] Build sanitized tests: " ${STX_ENABLE_BACKTRACE})

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
      add_compile_options("-fsanitize=address,undefined,thread")
    endif()

  endif()
endif()

# ===============================================
#
# === Configuration Definitions and Platform Support
#
# ===============================================

# remove this if(STX_VISIBLE_PANIC_HOOK) list(APPEND STX_COMPILER_DEFS
# "STX_VISIBLE_PANIC_HOOK") endif()

# if(STX_ENABLE_PANIC_BACKTRACE) list(APPEND STX_COMPILER_DEFS
# "STX_ENABLE_PANIC_BACKTRACE") endif()

# if(STX_ENABLE_BACKTRACE)
# TODO(lamarrr): check platform support endif()

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

# ===============================================
#
# === Library Setup
#
# ===============================================
# if(STX_ENABLE_BACKTRACE) target_link_libraries(stx absl::stacktrace
# absl::symbolize) endif()

# if(ANDROID) target_link_libraries(stx ${LibAtomic}) endif()

# find . -regex '.*\.\(cpp\|h\|cc\|cxx\)' -exec clang-format -style=file -i {}
# \; clang-format-all
