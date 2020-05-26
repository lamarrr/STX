/**
 * @file config.h
 * @author Basit Ayantunde <rlamarrr@gmail.com>
 * @brief
 * @version  0.1
 * @date 2020-05-22
 *
 * @copyright Copyright (c) 2020
 *
 */

#pragma once

#include <version>

/*********************** COMPILERS ***********************/

#if defined( \
    __GNUC__)  ///  any compiler that implements the GNU compiler extensions
#define STX_COMPILER_GNUC
#endif

#if defined(__clang__)
#define STX_COMPILER_CLANG
#endif

#if defined(_MSC_VER)
#define STX_COMPILER_MSVC
#endif

#if defined(__EMSCRIPTEN__)
#define STX_COMPILER_EMSCRIPTEN
#endif

#if defined(__NVCC__)
#define STX_COMPILER_NVCC
#endif

#if defined(__CC_ARM)
#define STX_COMPILER_ARM
#endif

#if defined(__INTEL_COMPILER) || defined(__ICL)
#define STX_COMPILER_INTEL
#endif

#if defined(__MINGW32__) || defined(__MINGW64__)
#define STX_COMPILER_MINGW
#endif

/*********************** OPERATING SYSTEMS ***********************/

#if defined(_WIN32) || defined(_WIN64) || defined(__WIN32__) || \
    defined(WIN32)  /// Any Windows
// you can actually compile for win64 in win32 mode, so checking for win64 is
// somehow redundant
#define STX_OS_WINDOWS
#endif

#if defined(__unix__)
#define STX_OS_UNIX
#endif

#if defined(__linux__)  /// Linux and variants like Android
#define STX_OS_LINUX
#endif

#if defined(__gnu_linux__)
#define STX_OS_GNU_LINUX  /// Linux OS with GNU facilities, unlike Android
#endif

#if defined(__ANDROID__)  /// Android, Also infers STX_OS_LINUX
#define STX_OS_ANDROID
#endif

#if defined(__APPLE__)  /// All apple OSs
#define STX_OS_APPLE
#endif

#if defined(__APPLE__) && defined(__MACH__)  /// Mac OS X
#define STX_OS_APPLE
#endif

// TODO: add Apple targets from TargetConditional.h

#if defined(__wasi__)  /// WebAssembly System Interface
#define STX_OS_WASI
#endif

#if defined(__CYGWIN__)  // Cygwin environment
#define STX_OS_CYGWIN
#endif

#if defined(__Fuchsia__)  /// Fuchsia
#define STX_OS_FUCHSIA
#endif

/*********************** ARCHITECTURES ***********************/

#if defined(__i386__) || defined(__i386) || defined(_X86_) || \
    defined(_M_IX86) || defined(_M_I86)
#define STX_ARCH_X86  /// X86
#endif

#if defined(__x86_64__) || defined(__x86_64) || defined(_M_X64) || \
    defined(_M_AMD64) || defined(__amd64) || defined(__amd64__)
#define STX_ARCH_X64  /// X64
#endif

#if defined(__arm__) || defined(_M_ARM)
#define STX_ARCH_ARM32  /// ARM
#endif

#if defined(__aarch64__)
#define STX_ARCH_ARM64  /// ARM64
#endif

#if defined(__XTENSA__)
#define STX_ARCH_XTENSA  /// Xtensa
#endif

#if defined(__mips__) || defined(__mips) || defined(mips) || defined(__MIPS__)
#define STX_ARCH_MIPS  // MIPS
#endif

#if defined(__riscv) || defined(__riscv__)  /// RISC-V
#define STX_ARCH_RISV
#endif

/************ FEATURE AND LIBRARY REQUIREMENTS ************/

#if __has_include(<source_location>)
#define STX_STABLE_LIB_SOURCE_LOCATION
#else
#if __has_include(<experimental/source_location>)
// ok
#else
#error C++20 Source location library is not available on this toolchain.  Please upgrade to a newer version.
#endif
#endif

#if !defined(__cpp_concepts) || (__cpp_concepts < 201907L)
#error 2019/07 version of concepts is not supported on this compiler.  Please upgrade to a newer version.
#endif

/*********************** UTILITY MACROS ***********************/

// also used for hiding static variables and hookable functions that should not
// be touched but should reside in the binary
// GNUC doesn't mean GCC!, it's also present in clang
#if __has_cpp_attribute(gnu::always_inline)
#define STX_FORCE_INLINE [[gnu::always_inline]] inline
#else
#ifdef _MSC_VER
#define STX_FORCE_INLINE __forceinline
#else
#if defined(__NVCC__)
#define STX_FORCE_INLINE __forcinline__
#else
#define STX_FORCE_INLINE inline
#endif
#endif
#endif

/*********************** ATTRIBUTE REQUIREMENTS ***********************/

#if !__has_cpp_attribute(nodiscard)
#error C++ attribute `nodiscard` is not available on this compiler. Important unused function results will not raise a warning. Please upgrade to a newer version.
#endif

#if !__has_cpp_attribute(deprecated)
#error C++ attribute `deprecated` is not available on this compiler. Please upgrade to a newer version.
#endif

#if !__has_cpp_attribute(noreturn)
#error C++ function attribute `noreturn` is not available on this compiler. Please upgrade to a newer version.
#endif

/*********************** SHARED LIBRARY SUPPORT ***********************/

#if defined(STX_OS_WINDOWS) || defined(STX_OS_CYGWIN)
#define STX_IMPORT __declspec(dllimport)
#define STX_EXPORT __declspec(dllexport)
#define STX_LOCAL
#else
#if __has_cpp_attribute(gnu::visibility)
#define STX_IMPORT [[gnu::visibility("default")]]
#define STX_EXPORT [[gnu::visibility("default")]]
#define STX_LOCAL [[gnu::visibility("hidden")]]
#else
#define STX_IMPORT
#define STX_EXPORT
#define STX_LOCAL
#endif
#endif

// TODO(lamarrr): Add binary formats, ELF, WASM, PE