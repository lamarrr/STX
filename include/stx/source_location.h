/**
 * @file source_location.h
 * @author Basit Ayantunde <rlamarrr@gmail.com>
 * @date 2020-06-04
 *
 * @copyright MIT License
 *
 * Copyright (c) 2020 Basit Ayantunde
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */

#pragma once

#include <cstdint>

#include "stx/config.h"

STX_BEGIN_NAMESPACE

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
      uint_least32_t line = __builtin_LINE(),
#else
      uint_least32_t line = 0,
#endif

#if STX_HAS_BUILTIN(COLUMN)
      uint_least32_t column = __builtin_COLUMN()
#else
      uint_least32_t column = 0
#endif
          ) noexcept {
    SourceLocation loc{};
    loc.line_ = line;
    loc.column_ = column;
    loc.file_ = file;
    loc.func_ = func;
    return loc;
  }

  // implementation-defined
  constexpr SourceLocation() noexcept
      : line_(), column_(), file_("\0"), func_("\0") {}
  constexpr SourceLocation(SourceLocation const& other) noexcept = default;
  constexpr SourceLocation(SourceLocation && other) noexcept = default;
  constexpr SourceLocation& operator=(SourceLocation const& other) noexcept =
      default;
  constexpr SourceLocation& operator=(SourceLocation&& other) noexcept =
      default;
  ~SourceLocation() noexcept = default;

  /// return the column number represented by this object
  constexpr uint_least32_t column() const noexcept { return column_; }

  /// return the line number represented by this object
  constexpr uint_least32_t line() const noexcept { return line_; }

  /// return the file name represented by this object
  constexpr const char* file_name() const noexcept { return file_; }

  /// return the name of the function represented by this object, if any
  constexpr const char* function_name() const noexcept { return func_; }

 private:
  uint_least32_t line_;
  uint_least32_t column_;
  const char* file_;
  const char* func_;
};

STX_END_NAMESPACE
