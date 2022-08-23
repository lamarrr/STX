/**
 * @file option_result.h
 * @author Basit Ayantunde <rlamarrr@gmail.com>
 * @date 2020-06-05
 *
 * @copyright MIT License
 *
 * Copyright (c) 2020-2022 Basit Ayantunde
 *
 */

/// @file
///
/// Error Propagation Macros
///
///

#pragma once

#include <type_traits>

#include "stx/config.h"
#include "stx/option.h"
#include "stx/option_result/impl/try.h"

#define STX_TRY_SOME_IMPL_(STX_ARG_UNIQUE_PLACEHOLDER, qualifier_identifier, \
                           ...)                                              \
  static_assert(!::std::is_const_v<decltype((__VA_ARGS__))>,                 \
                "the expression: ' " #__VA_ARGS__                            \
                " ' evaluates to a const and is not mutable");               \
  static_assert(!::std::is_lvalue_reference_v<decltype((__VA_ARGS__))>,      \
                "the expression: ' " #__VA_ARGS__                            \
                " ' evaluates to an l-value reference, 'TRY_SOME' only "     \
                "accepts r-values "                                          \
                "and r-value references ");                                  \
  decltype((__VA_ARGS__))&& STX_ARG_UNIQUE_PLACEHOLDER = (__VA_ARGS__);      \
                                                                             \
  if (STX_ARG_UNIQUE_PLACEHOLDER.is_none()) {                                \
    return ::stx::None;                                                      \
  }                                                                          \
                                                                             \
  typename std::remove_reference_t<decltype((__VA_ARGS__))>::value_type      \
      qualifier_identifier =                                                 \
          STX_ARG_UNIQUE_PLACEHOLDER.unsafe_some_ref().move();

/// if `option_expr` evaluates to an `Option` containing a `None`, `TRY_SOME`
/// returns its `None` value.
///
/// `option_expr` must be an expression yielding an r-value (reference) of type
/// `Option`
#define TRY_SOME(qualifier_identifier, ...)                           \
  STX_TRY_SOME_IMPL_(                                                 \
      STX_WITH_UNIQUE_SUFFIX_(STX_TRY_SOME_PLACEHOLDER, __COUNTER__), \
      qualifier_identifier, __VA_ARGS__)
