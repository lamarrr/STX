/**
 * @file panic_helpers.h
 * @author Basit Ayantunde <rlamarrr@gmail.com>
 * @date 2020-04-22
 *
 * @copyright MIT License
 *
 * Copyright (c) 2020-2022 Basit Ayantunde
 *
 */

#pragma once

#include "stx/panic.h"
#include "stx/source_location.h"

STX_BEGIN_NAMESPACE

namespace impl
{

namespace option
{

/// panic helper for `Option<T>::expect()` when no value is present
[[noreturn]] inline void expect_value_failed(
    std::string_view msg, SourceLocation location = SourceLocation::current())
{
  panic(msg, location);
}

/// panic helper for `Option<T>::expect_none()` when a value is present
[[noreturn]] inline void expect_none_failed(
    std::string_view msg, SourceLocation location = SourceLocation::current())
{
  panic(msg, location);
}

/// panic helper for `Option<T>::unwrap()` when no value is present
[[noreturn]] inline void no_value(
    SourceLocation location = SourceLocation::current())
{
  panic("called `Option::unwrap()` on a `None` value", location);
}

/// panic helper for `Option<T>::value()` when no value is present
[[noreturn]] inline void no_lref(
    SourceLocation location = SourceLocation::current())
{
  panic("called `Option::value()` on a `None` value", location);
}

/// panic helper for `Option<T>::unwrap_none()` when a value is present
[[noreturn]] inline void no_none(
    SourceLocation location = SourceLocation::current())
{
  panic("called `Option::unwrap_none()` on a `Some` value", location);
}

}        // namespace option

namespace result
{

/// panic helper for `Result<T, E>::expect()` when no value is present
template <typename E>
[[noreturn]] inline void expect_value_failed(
    std::string_view msg, E const &err,
    SourceLocation location = SourceLocation::current())
{
  panic(msg, err, location);
}

/// panic helper for `Result<T, E>::expect_err()` when a value is present
[[noreturn]] inline void expect_err_failed(
    std::string_view msg, SourceLocation location = SourceLocation::current())
{
  panic(msg, location);
}

/// panic helper for `Result<T, E>::unwrap()` when no value is present
template <typename E>
[[noreturn]] inline void no_value(
    E const &err, SourceLocation location = SourceLocation::current())
{
  panic("called `Result::unwrap()` on an `Err` value", err, location);
}

/// panic helper for `Result<T, E>::value()` when no value is present
template <typename E>
[[noreturn]] inline void no_lref(
    E const &err, SourceLocation location = SourceLocation::current())
{
  panic("called `Result::value()` on an `Err` value", err, location);
}

/// panic helper for `Result<T, E>::unwrap_err()` when a value is present
[[noreturn]] inline void no_err(
    SourceLocation location = SourceLocation::current())
{
  panic("called `Result::unwrap_err()` on an `Ok` value", location);
}

/// panic helper for `Result<T, E>::err()` when no value is present
[[noreturn]] inline void no_err_lref(
    SourceLocation location = SourceLocation::current())
{
  panic("called `Result::err()` on an `Ok` value", location);
}

}        // namespace result
}        // namespace impl

STX_END_NAMESPACE
