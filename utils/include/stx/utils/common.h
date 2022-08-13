/**
 * @file common.h
 * @author Basit Ayantunde <rlamarrr@gmail.com>
 * @date 2020-04-11
 *
 * @copyright MIT License
 *
 * Copyright (c) 2020-2022 Basit Ayantunde
 *
 */

#pragma once

#include <functional>
#include <type_traits>

#include "stx/config.h"

STX_BEGIN_NAMESPACE

template <typename Fn, typename... Args>
using invoke_result = typename std::invoke_result<Fn, Args...>::type;

template <typename Fn, typename... Args>
constexpr bool invocable = std::is_invocable<Fn, Args...>::value;

template <typename T>
constexpr bool movable =
    std::is_object<T>::value&& std::is_move_constructible<T>::value&&
        std::is_assignable<T&, T>::value&& std::is_swappable<T>::value;

template <typename T>
constexpr bool copy_constructible = std::is_copy_constructible<T>::value;

template <typename T>
constexpr bool default_constructible = std::is_default_constructible<T>::value;

template <typename From, typename To>
constexpr bool convertible = std::is_convertible<From, To>::value;

namespace impl {
template <typename T, typename Cmp = T, typename = void>
struct equality_comparable_impl : std::false_type {};

template <typename T, typename Cmp>
struct equality_comparable_impl<
    T, Cmp,
    typename std::enable_if_t<
        true,
        decltype((std::declval<std::remove_reference_t<T> const&>() ==
                  std::declval<std::remove_reference_t<Cmp> const&>()) &&
                     (std::declval<std::remove_reference_t<T> const&>() !=
                      std::declval<std::remove_reference_t<Cmp> const&>()),
                 (void)0)>> : std::true_type {};

}  // namespace impl

/// Checks if the type has a compatible 'operator ==' and 'operator!='
template <typename T, typename Cmp = T>
constexpr bool equality_comparable =
    impl::equality_comparable_impl<T, Cmp>::value;

template <typename T>
constexpr bool is_reference = std::is_reference<T>::value;

/// `Ref` is an alias for `std::reference_wrapper`
template <typename T>
using Ref = std::reference_wrapper<T>;

STX_END_NAMESPACE
