/**
 * @file source_location.h
 * @author Basit Ayantunde <rlamarrr@gmail.com>
 * @brief
 * @version  0.1
 * @date 2020-06-04
 *
 * @copyright Copyright (c) 2020
 *
 */

#pragma once

#if __has_include(<source_location>)
#include <source_location>
#define STX_HAS_SOURCE_LOCATION
#else
#if __has_include(<experimental/source_location>)
#include <experimental/source_location>
#define STX_HAS_EXPERIMENTAL_SOURCE_LOCATION
#endif

namespace stx {

#ifdef STX_HAS_SOURCE_LOCATION
using SourceLocation = std::source_location;
#else
#ifdef STX_HAS_EXPERIMENTAL_SOURCE_LOCATION
using SourceLocation = std::experimental::source_location;
#else

// we use this if the source location library is not available. It's equivalent
// to GCC's implementation
[[nodiscard]] struct SourceLocation {
  static constexpr SourceLocation current(

#if STX_HAS_BUILTIN(FILE)
      const char* file = __builtin_FILE(),
#else
      const char* file = "unknown",
#endif

#if STX_HAS_BUILTIN(FUNCTION)
      const char* func = __builtin_FUNCTION(),
#else
      const char* func = "unknown",
#endif

#if STX_HAS_BUILTIN(LINE)
      uint32_t line = __builtin_LINE(),
#else
      uint32_t line = 0,
#endif

#if STX_HAS_BUILTIN(COLUMN)
      uint32_t column = __builtin_COLUMN()
#else
      uint32_t column = 0
#endif
          ) noexcept {
    SourceLocation loc;
    loc.line_ = line;
    loc.column_ = column;
    loc.file_ = file;
    loc.func_ = func;
    return loc;
  }

  constexpr SourceLocation() noexcept = default;
  constexpr SourceLocation(const SourceLocation& other) noexcept = default;
  constexpr SourceLocation(SourceLocation&& other) noexcept = default;
  constexpr ~SourceLocation() const noexcept = default;

  constexpr uint32_t column() const noexcept { return column_; }
  constexpr uint32_t line() const noexcept { return line_; }
  constexpr const char* file_name() const noexcept { return file_; }
  constexpr const char* function_name() const noexcept { return func_; }

 private:
  uint32_t line_;
  uint32_t column_;
  const char* file_;
  const char* func_;
};

#endif
#endif
#endif
}
