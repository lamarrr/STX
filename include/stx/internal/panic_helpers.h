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
#ifndef STX_INTERNAL_PANIC_HELPERS_H_
#define STX_INTERNAL_PANIC_HELPERS_H_

#include "stx/common.h"
#include "stx/panic.h"

namespace stx {
namespace internal {

using namespace std::string_view_literals;  // NOLINT

namespace option {
// panic helper for `Option<T>::expect()` when no value is present
[[noreturn]] inline void expect_value_failed(
    std::string_view msg, SourceLocation location = SourceLocation::current()) {
  stx::panic(std::move(msg), std::move(location));
}

// panic helper for `Option<T>::expect_none()` when a value is present
[[noreturn]] inline void expect_none_failed(
    std::string_view msg, auto const& value,
    SourceLocation location = SourceLocation::current()) {
  (void)value;
  stx::panic(std::move(msg), std::move(location));
}

// panic helper for `Option<T>::unwrap()` when no value is present
[[noreturn]] inline void no_value(
    SourceLocation location = SourceLocation::current()) {
  stx::panic("called `Option::unwrap()` on a `None` value"sv,
             std::move(location));
}

// panic helper for `Option<T>::unwrap_none()` when a value is present
[[noreturn]] inline void no_none(
    auto const& value, SourceLocation location = SourceLocation::current()) {
  (void)value;
  stx::panic("called `Option::unwrap_none()` on a `Some` value"sv,
             std::move(location));
}

};  // namespace option

namespace result {

// panic helper for `Result<T, E>::expect()` when no value is present
[[noreturn]] inline void expect_value_failed(
    std::string_view msg, auto const& err,
    SourceLocation location = SourceLocation::current()) {
  (void)err;
  stx::panic(std::move(msg), std::move(location));
}

// panic helper for `Result<T, E>::expect_err()` when a value is present
[[noreturn]] inline void expect_err_failed(
    std::string_view msg, auto const& value,
    SourceLocation location = SourceLocation::current()) {
  (void)value;
  stx::panic(std::move(msg), std::move(location));
}

// panic helper for `Result<T, E>::unwrap()` when no value is present
[[noreturn]] inline void no_value(
    auto const& err, SourceLocation location = SourceLocation::current()) {
  (void)err;
  stx::panic("called `Result::unwrap()` on an `Err` value"sv,
             std::move(location));
}

//// panic helper for `Result<T, E>::unwrap_err()` when a value is present
[[noreturn]] inline void no_error(
    auto const& value, SourceLocation location = SourceLocation::current()) {
  (void)value;
  stx::panic("called `Result::unwrap_err()` on an `Ok` value"sv,
             std::move(location));
}

};      // namespace result
};      // namespace internal
};      // namespace stx
#endif  // STX_INTERNAL_PANIC_HELPERS_H_
