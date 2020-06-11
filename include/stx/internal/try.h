/**
 * @file option_result.h
 * @author Basit Ayantunde <rlamarrr@gmail.com>
 * @date 2020-06-05
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

/// @file
///
/// Error Propagation Macros
///
///

#pragma once

/// @cond

STX_BEGIN_NAMESPACE

template <typename Tp, typename Er>
STX_FORCE_INLINE Tp&& internal::result::unsafe_value_move(
    Result<Tp, Er>& result) {
  return std::move(result.value_ref_());
}

template <typename Tp, typename Er>
STX_FORCE_INLINE Er&& internal::result::unsafe_err_move(
    Result<Tp, Er>& result) {
  return std::move(result.err_ref_());
}

template <typename Tp>
STX_FORCE_INLINE Tp&& internal::option::unsafe_value_move(Option<Tp>& option) {
  return std::move(option.value_ref_());
}

STX_END_NAMESPACE

/// @endcond

#define STX_TRY__UTIL_JOIN_(x, y) x##_##y
#define STX_WITH_UNIQUE_SUFFIX_(x, y) STX_TRY__UTIL_JOIN_(x, y)

#define STX_TRY_OK_IMPL_(STX_ARG_UNIQUE_PLACEHOLDER, qualifier_identifier,     \
                         result_expr)                                          \
  static_assert(!::std::is_const_v<decltype((result_expr))>,                   \
                "the expression: ' " #result_expr                              \
                " ' evaluates to a const and is not mutable");                 \
  static_assert(                                                               \
      !::std::is_lvalue_reference_v<decltype((result_expr))>,                  \
      "the expression: ' " #result_expr                                        \
      " ' evaluates to an l-value reference, 'TRY_OK' only accepts r-values "  \
      "and r-value references ");                                              \
  decltype((result_expr))&& STX_ARG_UNIQUE_PLACEHOLDER = (result_expr);        \
                                                                               \
  if (STX_ARG_UNIQUE_PLACEHOLDER.is_err())                                     \
    return ::stx::Err<decltype((result_expr))::error_type>(                    \
        ::stx::internal::result::unsafe_err_move(STX_ARG_UNIQUE_PLACEHOLDER)); \
                                                                               \
  decltype((result_expr))::value_type qualifier_identifier =                   \
      ::stx::internal::result::unsafe_value_move(STX_ARG_UNIQUE_PLACEHOLDER);

#define STX_TRY_SOME_IMPL_(STX_ARG_UNIQUE_PLACEHOLDER, qualifier_identifier, \
                           option_expr)                                      \
  static_assert(!::std::is_const_v<decltype((option_expr))>,                 \
                "the expression: ' " #option_expr                            \
                " ' evaluates to a const and is not mutable");               \
  static_assert(!::std::is_lvalue_reference_v<decltype((option_expr))>,      \
                "the expression: ' " #option_expr                            \
                " ' evaluates to an l-value reference, 'TRY_SOME' only "     \
                "accepts r-values "                                          \
                "and r-value references ");                                  \
  decltype((option_expr))&& STX_ARG_UNIQUE_PLACEHOLDER = (option_expr);      \
                                                                             \
  if (STX_ARG_UNIQUE_PLACEHOLDER.is_none()) return ::stx::None;              \
                                                                             \
  decltype((option_expr))::value_type qualifier_identifier =                 \
      ::stx::internal::option::unsafe_value_move(STX_ARG_UNIQUE_PLACEHOLDER);

/// if `result_expr` is a `Result` containing an error, `TRY_OK` returns its
/// `Err` value.
///
/// `result_expr` must be an expression yielding an r-value (reference) of type
/// `Result`
#define TRY_OK(qualifier_identifier, result_expr)                   \
  STX_TRY_OK_IMPL_(                                                 \
      STX_WITH_UNIQUE_SUFFIX_(STX_TRY_OK_PLACEHOLDER, __COUNTER__), \
      qualifier_identifier, result_expr)

/// if `option_expr` evaluates to an `Option` containing a `None`, `TRY_SOME`
/// returns its `None` value.
///
/// `option_expr` must be an expression yielding an r-value (reference) of type
/// `Option`
#define TRY_SOME(qualifier_identifier, option_expr)                   \
  STX_TRY_SOME_IMPL_(                                                 \
      STX_WITH_UNIQUE_SUFFIX_(STX_TRY_SOME_PLACEHOLDER, __COUNTER__), \
      qualifier_identifier, option_expr)

// Coroutines
// this feature is experimental and not widely tested yet
#if defined(__cpp_coroutines) || defined(__cpp_lib_coroutine)

#define STX_CO_TRY_OK_IMPL_(STX_ARG_UNIQUE_PLACEHOLDER, qualifier_identifier,  \
                            result_expr)                                       \
  static_assert(!::std::is_const_v<decltype((result_expr))>,                   \
                "the expression: ' " #result_expr                              \
                " ' evaluates to a const and is not mutable");                 \
  static_assert(!::std::is_lvalue_reference_v<decltype((result_expr))>,        \
                "the expression: ' " #result_expr                              \
                " ' evaluates to an l-value reference, 'CO_TRY_OK' only "      \
                "accepts r-values "                                            \
                "and r-value references ");                                    \
  decltype((result_expr))&& STX_ARG_UNIQUE_PLACEHOLDER = (result_expr);        \
                                                                               \
  if (STX_ARG_UNIQUE_PLACEHOLDER.is_err())                                     \
    co_return ::stx::Err<decltype((result_expr))::error_type>(                 \
        ::stx::internal::result::unsafe_err_move(STX_ARG_UNIQUE_PLACEHOLDER)); \
                                                                               \
  decltype((result_expr))::value_type qualifier_identifier =                   \
      ::stx::internal::result::unsafe_value_move(STX_ARG_UNIQUE_PLACEHOLDER);

#define STX_CO_TRY_SOME_IMPL_(STX_ARG_UNIQUE_PLACEHOLDER,                   \
                              qualifier_identifier, option_expr)            \
  static_assert(!::std::is_const_v<decltype((option_expr))>,                \
                "the expression: ' " #option_expr                           \
                " ' evaluates to a const and is not mutable");              \
  static_assert(!::std::is_lvalue_reference_v<decltype((option_expr))>,     \
                "the expression: ' " #option_expr                           \
                " ' evaluates to an l-value reference, 'CO_TRY_SOME' only " \
                "accepts r-values "                                         \
                "and r-value references ");                                 \
  decltype((option_expr))&& STX_ARG_UNIQUE_PLACEHOLDER = (option_expr);     \
                                                                            \
  if (STX_ARG_UNIQUE_PLACEHOLDER.is_none()) co_return ::stx::None;          \
                                                                            \
  decltype((option_expr))::value_type qualifier_identifier =                \
      ::stx::internal::option::unsafe_value_move(STX_ARG_UNIQUE_PLACEHOLDER);

/// COROUTINES ONLY. if `result_expr` is `Result` containing an error,
/// `CO_TRY_OK` co-returns its `Err` value.
///
/// `result_expr` must be an expression yielding an r-value (reference) of type
/// `Result`
#define CO_TRY_OK(qualifier_identifier, result_expr)                   \
  STX_CO_TRY_OK_IMPL_(                                                 \
      STX_WITH_UNIQUE_SUFFIX_(STX_CO_TRY_OK_PLACEHOLDER, __COUNTER__), \
      qualifier_identifier, result_expr)

/// COROUTINES ONLY. if `option_expr` evaluates to an `Option` containing a
/// `None`, `TRY_SOME` co-returns its `None` value.
///
/// `option_expr` must be an expression yielding an r-value (reference) of type
/// `Option`
#define CO_TRY_SOME(qualifier_identifier, option_expr)                   \
  STX_CO_TRY_SOME_IMPL_(                                                 \
      STX_WITH_UNIQUE_SUFFIX_(STX_CO_TRY_SOME_PLACEHOLDER, __COUNTER__), \
      qualifier_identifier, option_expr)

#endif
