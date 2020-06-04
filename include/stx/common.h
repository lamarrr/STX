/**
 * @file common.h
 * @author Basit Ayantunde <rlamarrr@gmail.com>
 * @brief
 * @version  0.1
 * @date 2020-04-11
 *
 * @copyright Copyright (c) 2020
 *
 */

#pragma once

#include <functional>
#include <iterator>
#include <type_traits>

#include "stx/config.h"

namespace stx {
// NOTE: these implementations will be replaced by standard versions in future
// release, most toolchains don't implement these yet

template <typename Fn, typename... Args>
using invoke_result = typename std::invoke_result_t<Fn, Args...>;

template <typename T, typename Cmp = T, typename = void>
struct is_equality_comparable : std::false_type {};

template <typename T, typename Cmp>
struct is_equality_comparable<
    T, Cmp,
    typename std::enable_if_t<
        true,
        decltype((std::declval<std::remove_reference_t<T> const&>() ==
                  std::declval<std::remove_reference_t<Cmp> const&>()) &&
                     (std::declval<std::remove_reference_t<T> const&>() !=
                      std::declval<std::remove_reference_t<Cmp> const&>()),
                 (void)0)>> : std::true_type {};

template <typename T, typename Cmp = T>
constexpr bool is_equality_comparable_v = is_equality_comparable<T, Cmp>::value;

template <typename T>
using Ref = std::reference_wrapper<T>;

template <typename T>
using ConstRef =
    std::reference_wrapper<std::add_const_t<std::remove_reference_t<T>>>;

template <typename T>
using MutRef =
    std::reference_wrapper<std::remove_const_t<std::remove_reference_t<T>>>;

#if defined(__cpp_concepts)
#if __cpp_concepts > 201907L
template <typename T, typename Base>
concept Impl = std::is_base_of_v<Base, T>;
#endif
#endif

}  // namespace stx
