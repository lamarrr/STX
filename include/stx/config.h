#pragma once

#include <version>

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
#error C++ attribute `deprecated` is not available on this compiler. Please upgrade to a newer version.
#endif

#if defined(_WIN32)
#define STX_WINDOWS
#else
#define STX_NOT_WINDOWS
#endif

#if __has_include(<pthreads.h>)
#define HAS_POSIX_THREADS
#endif
