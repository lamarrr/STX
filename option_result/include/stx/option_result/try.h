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
                         ...)                                                  \
  static_assert(!::std::is_const_v<decltype((__VA_ARGS__))>,                   \
                "the expression: ' " #__VA_ARGS__                              \
                " ' evaluates to a const and is not mutable");                 \
  static_assert(                                                               \
      !::std::is_lvalue_reference_v<decltype((__VA_ARGS__))>,                  \
      "the expression: ' " #__VA_ARGS__                                        \
      " ' evaluates to an l-value reference, 'TRY_OK' only accepts r-values "  \
      "and r-value references ");                                              \
  decltype((__VA_ARGS__))&& STX_ARG_UNIQUE_PLACEHOLDER = (__VA_ARGS__);        \
                                                                               \
  if (STX_ARG_UNIQUE_PLACEHOLDER.is_err())                                     \
    return ::stx::Err<typename std::remove_reference_t<decltype(               \
        (__VA_ARGS__))>::error_type>(                                          \
        ::stx::internal::result::unsafe_err_move(STX_ARG_UNIQUE_PLACEHOLDER)); \
                                                                               \
  typename std::remove_reference_t<decltype((__VA_ARGS__))>::value_type        \
      qualifier_identifier = ::stx::internal::result::unsafe_value_move(       \
          STX_ARG_UNIQUE_PLACEHOLDER);

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
  if (STX_ARG_UNIQUE_PLACEHOLDER.is_none()) return ::stx::None;              \
                                                                             \
  typename std::remove_reference_t<decltype((__VA_ARGS__))>::value_type      \
      qualifier_identifier = ::stx::internal::option::unsafe_value_move(     \
          STX_ARG_UNIQUE_PLACEHOLDER);

/// if `result_expr` is a `Result` containing an error, `TRY_OK` returns its
/// `Err` value.
///
/// `result_expr` must be an expression yielding an r-value (reference) of type
/// `Result`
#define TRY_OK(qualifier_identifier, ...)                           \
  STX_TRY_OK_IMPL_(                                                 \
      STX_WITH_UNIQUE_SUFFIX_(STX_TRY_OK_PLACEHOLDER, __COUNTER__), \
      qualifier_identifier, __VA_ARGS__)

/// if `option_expr` evaluates to an `Option` containing a `None`, `TRY_SOME`
/// returns its `None` value.
///
/// `option_expr` must be an expression yielding an r-value (reference) of type
/// `Option`
#define TRY_SOME(qualifier_identifier, ...)                           \
  STX_TRY_SOME_IMPL_(                                                 \
      STX_WITH_UNIQUE_SUFFIX_(STX_TRY_SOME_PLACEHOLDER, __COUNTER__), \
      qualifier_identifier, __VA_ARGS__)
