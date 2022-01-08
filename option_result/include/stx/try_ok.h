/**
 * @file option_result.h
 * @author Basit Ayantunde <rlamarrr@gmail.com>
 * @date 2020-06-05
 *
 * @copyright MIT License
 *
 * Copyright (c) 2020-2021 Basit Ayantunde
 *
 */

/// @file
///
/// Error Propagation Macros
///
///

#pragma once

#include <type_traits>
#include <utility>

#include "stx/config.h"
#include "stx/option_result/impl/try.h"
#include "stx/result.h"

#define STX_TRY_OK_IMPL_(STX_ARG_UNIQUE_PLACEHOLDER, qualifier_identifier,    \
                         ...)                                                 \
  static_assert(!::std::is_const_v<decltype((__VA_ARGS__))>,                  \
                "the expression: ' " #__VA_ARGS__                             \
                " ' evaluates to a const and is not mutable");                \
  static_assert(                                                              \
      !::std::is_lvalue_reference_v<decltype((__VA_ARGS__))>,                 \
      "the expression: ' " #__VA_ARGS__                                       \
      " ' evaluates to an l-value reference, 'TRY_OK' only accepts r-values " \
      "and r-value references ");                                             \
  decltype((__VA_ARGS__))&& STX_ARG_UNIQUE_PLACEHOLDER = (__VA_ARGS__);       \
                                                                              \
  if (STX_ARG_UNIQUE_PLACEHOLDER.is_err()) {                                  \
    return std::move(STX_ARG_UNIQUE_PLACEHOLDER.unsafe_err_ref());            \
  }                                                                           \
                                                                              \
  typename std::remove_reference_t<decltype((__VA_ARGS__))>::value_type       \
      qualifier_identifier =                                                  \
          STX_ARG_UNIQUE_PLACEHOLDER.unsafe_ok_ref().move();

/// if `result_expr` is a `Result` containing an error, `TRY_OK` returns its
/// `Err` value.
///
/// `result_expr` must be an expression yielding an r-value (reference) of type
/// `Result`
#define TRY_OK(qualifier_identifier, ...)                           \
  STX_TRY_OK_IMPL_(                                                 \
      STX_WITH_UNIQUE_SUFFIX_(STX_TRY_OK_PLACEHOLDER, __COUNTER__), \
      qualifier_identifier, __VA_ARGS__)
