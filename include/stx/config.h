/**
 * @file config.h
 * @author Basit Ayantunde <rlamarrr@gmail.com>
 * @date 2020-05-22
 *
 * @copyright MIT License
 *
 * Copyright (c) 2020-2022 Basit Ayantunde
 *
 */

#pragma once

/// configuration macro
#define STX_CFG(param, value) STX_##param##_##value

/*********************** COMPILERS ***********************/

#if defined(__GNUC__)        //  any compiler that implements the GNU compiler
                             //  extensions
#  define STX_COMPILER_GNUC 1
#else
#  define STX_COMPILER_GNUC 0
#endif

#if defined(__clang__)
#  define STX_COMPILER_CLANG 1
#else
#  define STX_COMPILER_CLANG 0
#endif

#if defined(_MSC_VER)
#  define STX_COMPILER_MSVC 1
#else
#  define STX_COMPILER_MSVC 0
#endif

#if defined(__EMSCRIPTEN__)
#  define STX_COMPILER_EMSCRIPTEN 1
#else
#  define STX_COMPILER_EMSCRIPTEN 0
#endif

#if defined(__NVCC__)
#  define STX_COMPILER_NVCC 1
#else
#  define STX_COMPILER_NVCC 0
#endif

#if defined(__CC_ARM)
#  define STX_COMPILER_ARM 1
#else
#  define STX_COMPILER_ARM 0
#endif

#if defined(__INTEL_COMPILER) || defined(__ICL)
#  define STX_COMPILER_INTEL 1
#else
#  define STX_COMPILER_INTEL 0
#endif

#if defined(__MINGW32__) || defined(__MINGW64__)
#  define STX_COMPILER_MINGW 1
#else
#  define STX_COMPILER_MINGW 0
#endif

/*********************** OPERATING SYSTEMS ***********************/

#if defined(_WIN32) || defined(_WIN64) || defined(__WIN32__) || \
    defined(WIN32)        // Any Windows
#  define STX_OS_WINDOWS 1
#else
#  define STX_OS_WINDOWS 0
#endif

#if defined(__unix__) || defined(unix)        // UNIX
#  define STX_OS_UNIX 1
#else
#  define STX_OS_UNIX 0
#endif

#if __has_include(<unistd.h>)        // Posix-compliant operating system
#  define STX_OS_POSIX 1
#else
#  define STX_OS_POSIX 0
#endif

#if defined(__linux__) || defined(__linux) || \
    defined(linux)        // Linux and variants like Android
#  define STX_OS_LINUX 1
#else
#  define STX_OS_LINUX 0
#endif

#if defined(__gnu_linux__)        // Linux OS with GNU facilities
#  define STX_OS_GNU_LINUX 1
#else
#  define STX_OS_GNU_LINUX 0
#endif

#if defined(__ANDROID__)        // Android, Also infers STX_OS_LINUX
#  define STX_OS_ANDROID 1
#else
#  define STX_OS_ANDROID 0
#endif

#if defined(__APPLE__)        // All Apple OSs
#  define STX_OS_APPLE 1

#  include <Availability.h>
#  include <TargetConditionals.h>

#else
#  define STX_OS_APPLE 0
#endif

#if defined(__APPLE__) && defined(__MACH__)        // Mac OS X
#  define STX_OS_MACOS 1
#else
#  define STX_OS_MACOS 0
#endif

// TODO(lamarrr): add Apple targets from TargetConditional.h

#if defined(__wasi__)        // WebAssembly System Interface
#  define STX_OS_WASI 1
#else
#  define STX_OS_WASI 0
#endif

#if defined(__CYGWIN__)        // Cygwin environment
#  define STX_OS_CYGWIN 1
#else
#  define STX_OS_CYGWIN 0
#endif

#if defined(__Fuchsia__)        // Fuchsia
#  define STX_OS_FUCHSIA 1
#else
#  define STX_OS_FUCHSIA 0
#endif

/*********************** ARCHITECTURES ***********************/

#if defined(__i386__) || defined(__i386) || defined(_X86_) || \
    defined(_M_IX86) || defined(_M_I86)        // X86
#  define STX_ARCH_X86 1
#else
#  define STX_ARCH_X86 0
#endif

#if defined(__x86_64__) || defined(__x86_64) || defined(_M_X64) || \
    defined(_M_AMD64) || defined(__amd64) || defined(__amd64__)        // X86_64
#  define STX_ARCH_X86_64 1
#else
#  define STX_ARCH_X86_64 0
#endif

#if defined(__arm__) || defined(_M_ARM)        // ARM
#  define STX_ARCH_ARM32 1
#else
#  define STX_ARCH_ARM32 0
#endif

#if defined(__aarch64__)        // ARM64
#  define STX_ARCH_ARM64 1
#else
#  define STX_ARCH_ARM64 0
#endif

#if defined(__XTENSA__)        // Xtensa
#  define STX_ARCH_XTENSA 1
#else
#  define STX_ARCH_XTENSA 0
#endif

#if defined(__mips__) || defined(__mips) || defined(mips) || \
    defined(__MIPS__)        // MIPS
#  define STX_ARCH_MIPS 1
#else
#  define STX_ARCH_MIPS 0
#endif

#if defined(__riscv) || defined(__riscv__)        // RISC-V
#  define STX_ARCH_RISCV 1
#else
#  define STX_ARCH_RISCV 0
#endif

/************ FEATURE AND LIBRARY REQUIREMENTS ************/

#if defined __has_builtin
#  define STX_HAS_BUILTIN(feature) __has_builtin(__builtin_##feature)
#else
#  define STX_HAS_BUILTIN(feature) 0
#endif

/*********************** ATTRIBUTE REQUIREMENTS ***********************/

#if defined(__has_cpp_attribute)
#  if !__has_cpp_attribute(nodiscard)
#    error C++ attribute `nodiscard` is not available on this compiler. Important unused function results will not raise a warning. Please upgrade to a newer version.
#  endif

#  if !__has_cpp_attribute(deprecated)
#    error C++ attribute `deprecated` is not available on this compiler. Please upgrade to a newer version.
#  endif

#  if !__has_cpp_attribute(noreturn)
#    error C++ function attribute `noreturn` is not available on this compiler. Please upgrade to a newer version.
#  endif
#endif

/*********************** BINARY FORMATS ***********************/

#if defined(__wasm__)        // Web Assembly
#  define STX_BINARY_WASW 1
#else
#  define STX_BINARY_WASW 0
#endif

#if defined(__ELF__)        // Executable and Linkable Formats
#  define STX_BINARY_ELF 1
#else
#  define STX_BINARY_ELF 0
#endif

#if STX_CFG(OS, WINDOWS)        // Windows Portable Executable
#  define STX_BINARY_EXE 1
#else
#  define STX_BINARY_EXE 0
#endif

/*********************** TOOLCHAINS ***********************/

#if defined(__llvm__)
#  define STX_TOOLCHAIN_LLVM 1
#else
#  define STX_TOOLCHAIN_LLVM 0
#endif

/*********************** SHARED LIBRARY (DLL) SUPPORT ***********************/

#if STX_CFG(OS, WINDOWS) || STX_CFG(OS, CYGWIN)
// symbols are hidden by default on windows DLLs
#  define STX_DLL_IMPORT __declspec(dllimport)
#  define STX_DLL_EXPORT __declspec(dllexport)
#  define STX_DLL_HIDDEN
#else
#  if STX_CFG(COMPILER, GNUC)
// symbols are visible by default on GNUC DLLs
#    define STX_DLL_IMPORT __attribute__((visibility("default")))
#    define STX_DLL_EXPORT __attribute__((visibility("default")))
#    define STX_DLL_HIDDEN __attribute__((visibility("hidden")))
#  else
#    define STX_DLL_IMPORT
#    define STX_DLL_EXPORT
#    define STX_DLL_HIDDEN
#  endif
#endif

/*********************** INLINING MACROS ***********************/

// also used for hiding static variables and hookable functions that should not
// be touched but should reside in the binary
// GNUC doesn't mean GCC!, it's also present in clang
#if STX_CFG(COMPILER, GNUC)
#  define STX_FORCE_INLINE __attribute__((always_inline)) inline
#else
#  if STX_CFG(COMPILER, MSVC)
#    define STX_FORCE_INLINE __forceinline
#  else
#    if STX_CFG(COMPILER, NVCC)
#      define STX_FORCE_INLINE __forceinline__ inline
#    else
#      define STX_FORCE_INLINE inline
#    endif
#  endif
#endif

/*********************** VERSIONING ***********************/

#define STX_VERSION v1
#define STX_VERSION_STRING "v1"

#define STX_BEGIN_NAMESPACE \
  namespace stx             \
  {                         \
  inline namespace v1       \
  {

#define STX_END_NAMESPACE \
  }                       \
  }
