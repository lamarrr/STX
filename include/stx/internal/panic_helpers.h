/**
 * @file panic_helpers.h
 * @author Basit Ayantunde <rlamarrr@gmail.com>
 * @date 2020-04-22
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

#include "stx/panic.h"

STX_BEGIN_NAMESPACE

namespace internal {

// Forced inline functions basically function like macros.

namespace option {

/// panic helper for `Option<T>::expect()` when no value is present
[[noreturn]] STX_FORCE_INLINE void expect_value_failed(
    std::string_view const& msg,
    SourceLocation const& location = SourceLocation::current()) noexcept {
  panic(msg, location);
}

/// panic helper for `Option<T>::expect_none()` when a value is present
[[noreturn]] STX_FORCE_INLINE void expect_none_failed(
    std::string_view const& msg,
    SourceLocation const& location = SourceLocation::current()) noexcept {
  panic(msg, location);
}

/// panic helper for `Option<T>::unwrap()` when no value is present
[[noreturn]] STX_FORCE_INLINE void no_value(
    SourceLocation const& location = SourceLocation::current()) noexcept {
  panic("called `Option::unwrap()` on a `None` value", location);
}

/// panic helper for `Option<T>::value()` when no value is present
[[noreturn]] STX_FORCE_INLINE void no_lref(
    SourceLocation const& location = SourceLocation::current()) noexcept {
  panic("called `Option::value()` on a `None` value", location);
}

/// panic helper for `Option<T>::unwrap_none()` when a value is present
[[noreturn]] STX_FORCE_INLINE void no_none(
    SourceLocation const& location = SourceLocation::current()) noexcept {
  panic("called `Option::unwrap_none()` on a `Some` value", location);
}

}  // namespace option

namespace result {

/// panic helper for `Result<T, E>::expect()` when no value is present
template <typename T>
[[noreturn]] STX_FORCE_INLINE void expect_value_failed(
    std::string_view const& msg, T const& err,
    SourceLocation const& location = SourceLocation::current()) noexcept {
  panic(msg, err, location);
}

/// panic helper for `Result<T, E>::expect_err()` when a value is present
[[noreturn]] STX_FORCE_INLINE void expect_err_failed(
    std::string_view const& msg,
    SourceLocation const& location = SourceLocation::current()) noexcept {
  panic(msg, location);
}

/// panic helper for `Result<T, E>::unwrap()` when no value is present
template <typename T>
[[noreturn]] STX_FORCE_INLINE void no_value(
    T const& err,
    SourceLocation const& location = SourceLocation::current()) noexcept {
  panic("called `Result::unwrap()` on an `Err` value", err, location);
}

/// panic helper for `Result<T, E>::value()` when no value is present
template <typename T>
[[noreturn]] STX_FORCE_INLINE void no_lref(
    T const& err,
    SourceLocation const& location = SourceLocation::current()) noexcept {
  panic("called `Result::value()` on an `Err` value", err, location);
}

/// panic helper for `Result<T, E>::unwrap_err()` when a value is present
[[noreturn]] STX_FORCE_INLINE void no_err(
    SourceLocation const& location = SourceLocation::current()) noexcept {
  panic("called `Result::unwrap_err()` on an `Ok` value", location);
}

/// panic helper for `Result<T, E>::err_value()` when no value is present
[[noreturn]] STX_FORCE_INLINE void no_err_lref(
    SourceLocation const& location = SourceLocation::current()) noexcept {
  panic("called `Result::err_value()` on an `Ok` value", location);
}

}  // namespace result
}  // namespace internal

STX_END_NAMESPACE
