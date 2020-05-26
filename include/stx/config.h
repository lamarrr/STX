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

/*********** BASE MINIMUM FEATURES AND LIBRARIES ************/
#if __has_include(<source_location>)
#define STX_STABLE_SOURCE_LOCATION
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

/*********************** COMPILERS ***********************/

// also used for hiding static variables and hookable functions that should not
// be touched but should reside in the ABI
// GNUC doesn't mean GCC!, it is also present in clang
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

#if defined _MSC_VER
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

/*********************** OPERATING SYSTEM ***********************/
#if defined(_WIN32)
#define STX_OS_WINDOWS
#endif

#if defined(__linux__)  // and variants like Android
#define STX_OS_LINUX
#endif

#if defined(__ANDROID__)
#define STX_OS_ANDROID
#endif

#if defined(__APPLE__)  //  (macOS and iOS)
#define STX_OS_APPLE
#endif

#if defined(__wasm__)  // WebAssembly
#define STX_OS_WASM
#endif

#if defined(__Fuchsia__)  // Fuchsia
#define STX_OS_FUCHSIA
#endif

/*********************** ARCHITECTURE ***********************/
#if defined(__x86_64__) || defined(__x86_64) || defined(_M_X64) || \
    defined(_M_AMD64) || defined(__amd64) || defined(__amd64__)
#define STX_ARCH_X86_64
#endif

#if defined(__i386__) || defined(__i386) || defined(_X86_) || \
    defined(_M_IX86) || defined(_M_I86)
#define STX_ARCH_X86
#endif

#if defined(__arm__) || defined(_M_ARM)
#define STX_ARCH_ARM32
#endif

#if defined(__aarch64__)
#define STX_ARCH_ARM64
#endif
