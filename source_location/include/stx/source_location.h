/**
 * @file source_location.h
 * @author Basit Ayantunde <rlamarrr@gmail.com>
 * @date 2020-06-04
 *
 * @copyright MIT License
 *
 * Copyright (c) 2020-2021 Basit Ayantunde
 *
 */

#pragma once

#include <cstdint>

#include "stx/config.h"

STX_BEGIN_NAMESPACE

#if STX_HAS_BUILTIN(FUNCTION)
#define STX_BUILTIN_FUNCTION() __builtin_FUNCTION()
#else
#define STX_BUILTIN_FUNCTION() "unknown function"
#endif

#if STX_HAS_BUILTIN(FILE)
#define STX_BUILTIN_FILE() __builtin_FILE()
#else
#define STX_BUILTIN_FILE() "unknown file"
#endif

#if STX_HAS_BUILTIN(LINE)
#define STX_BUILTIN_LINE() __builtin_LINE()
#else
#define STX_BUILTIN_LINE() 0
#endif

#if STX_HAS_BUILTIN(LINE)
#define STX_BUILTIN_COLUMN() __builtin_COLUMN()
#else
#define STX_BUILTIN_COLUMN() 0
#endif

///
/// The `SourceLocation`  class represents certain information about the source
/// code, such as file names, line numbers, and function names. Previously,
/// functions that desire to obtain this information about the call site (for
/// logging, testing, or debugging purposes) must use macros so that predefined
/// macros like `__LINE__` and `__FILE__` are expanded in the context of the
/// caller. The `SourceLocation` class provides a better alternative.
///
///
/// based on: https://en.cppreference.com/w/cpp/utility/source_location
///
// It's equivalent to GCC's implementation
struct [[nodiscard]] SourceLocation {
  static constexpr SourceLocation current(
      char const* file = STX_BUILTIN_FILE(),
      char const* func = STX_BUILTIN_FUNCTION(),
      uint_least32_t line = STX_BUILTIN_LINE(),
      uint_least32_t column = STX_BUILTIN_COLUMN()) {
    SourceLocation loc{};
    loc.line_ = line;
    loc.column_ = column;
    loc.file_ = file;
    loc.func_ = func;
    return loc;
  }

  constexpr SourceLocation() : line_{0}, column_{0}, file_{""}, func_{""} {}

  /// return the column number represented by this object
  constexpr uint_least32_t column() const { return column_; }

  /// return the line number represented by this object
  constexpr uint_least32_t line() const { return line_; }

  /// return the file name represented by this object
  constexpr char const* file_name() const { return file_; }

  /// return the name of the function represented by this object, if any
  constexpr char const* function_name() const { return func_; }

 private:
  uint_least32_t line_;
  uint_least32_t column_;
  char const* file_;
  char const* func_;
};

STX_END_NAMESPACE
