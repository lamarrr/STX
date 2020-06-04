/**
 * @file panic_helpers.h
 * @author Basit Ayantunde <rlamarrr@gmail.com>
 * @brief
 * @version  0.1
 * @date 2020-04-22
 *
 * @copyright Copyright (c) 2020
 *
 */

#pragma once

#include "stx/panic.h"

namespace stx {
namespace internal {

// Forced inline functions basically function like macros.

namespace option {
using namespace std::string_view_literals;  // NOLINT

/// panic helper for `Option<T>::expect()` when no value is present
[[noreturn]] STX_FORCE_INLINE void expect_value_failed(
    std::string_view&& msg,
    SourceLocation location = SourceLocation::current()) noexcept {
  stx::panic(std::forward<std::string_view&&>(msg), std::move(location));
}

/// panic helper for `Option<T>::expect_none()` when a value is present
template <typename T>
[[noreturn]] STX_FORCE_INLINE void expect_none_failed(
    std::string_view&& msg, T const& value,
    SourceLocation location = SourceLocation::current()) noexcept {
  stx::panic(std::forward<std::string_view&&>(msg), value, std::move(location));
}

/// panic helper for `Option<T>::unwrap()` when no value is present
[[noreturn]] STX_FORCE_INLINE void no_value(
    SourceLocation location = SourceLocation::current()) noexcept {
  stx::panic("called `Option::unwrap()` on a `None` value",
             std::move(location));
}

/// panic helper for `Option<T>::value()` when no value is present
[[noreturn]] STX_FORCE_INLINE void no_lref(
    SourceLocation location = SourceLocation::current()) noexcept {
  stx::panic("called `Option::value()` on a `None` value", std::move(location));
}

/// panic helper for `Option<T>::unwrap_none()` when a value is present
template <typename T>
[[noreturn]] STX_FORCE_INLINE void no_none(
    T const& value,
    SourceLocation location = SourceLocation::current()) noexcept {
  stx::panic("called `Option::unwrap_none()` on a `Some` value", value,
             std::move(location));
}

};  // namespace option

namespace result {
using namespace std::string_view_literals;  // NOLINT

/// panic helper for `Result<T, E>::expect()` when no value is present
template <typename T>
[[noreturn]] STX_FORCE_INLINE void expect_value_failed(
    std::string_view&& msg, T const& err,
    SourceLocation location = SourceLocation::current()) noexcept {
  stx::panic(std::forward<std::string_view&&>(msg), err, std::move(location));
}

/// panic helper for `Result<T, E>::expect_err()` when a value is present
template <typename T>
[[noreturn]] STX_FORCE_INLINE void expect_err_failed(
    std::string_view&& msg, T const& value,
    SourceLocation location = SourceLocation::current()) noexcept {
  stx::panic(std::forward<std::string_view&&>(msg), value, std::move(location));
}

/// panic helper for `Result<T, E>::unwrap()` when no value is present
template <typename T>
[[noreturn]] STX_FORCE_INLINE void no_value(
    T const& err,
    SourceLocation location = SourceLocation::current()) noexcept {
  stx::panic("called `Result::unwrap()` on an `Err` value"sv, err,
             std::move(location));
}

/// panic helper for `Result<T, E>::value()` when no value is present
template <typename T>
[[noreturn]] STX_FORCE_INLINE void no_lref(
    T const& err,
    SourceLocation location = SourceLocation::current()) noexcept {
  stx::panic("called `Result::value()` on an `Err` value"sv, err,
             std::move(location));
}

/// panic helper for `Result<T, E>::unwrap_err()` when a value is present
template <typename T>
[[noreturn]] STX_FORCE_INLINE void no_err(
    T const& value,
    SourceLocation location = SourceLocation::current()) noexcept {
  stx::panic("called `Result::unwrap_err()` on an `Ok` value"sv, value,
             std::move(location));
}

/// panic helper for `Result<T, E>::err_value()` when no value is present
template <typename T>
[[noreturn]] STX_FORCE_INLINE void no_err_lref(
    T const& value,
    SourceLocation location = SourceLocation::current()) noexcept {
  stx::panic("called `Result::err_value()` on an `Ok` value"sv, value,
             std::move(location));
}

};  // namespace result
};  // namespace internal
};  // namespace stx
