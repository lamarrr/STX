/**
 * @file common.h
 * @author Basit Ayantunde <rlamarrr@gmail.com>
 * @date 2020-04-11
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

/// Checks if the type has a compatible 'operator ==' and 'operator!='
template <typename T, typename Cmp = T>
constexpr bool equality_comparable = is_equality_comparable<T, Cmp>::value;

template <typename T>
constexpr bool is_reference = std::is_reference<T>::value;

/// `Ref` is an alias for `std::reference_wrapper`
/// `Ref` can be mutable and immutable depending on the const-qualifier for `T`
/// To offer stronger guarantees prefer `ConstRef` and `MutRef`
template <typename T>
using Ref = std::reference_wrapper<T>;

/// `ConstRef` is an always-const `Ref`.
template <typename T>
using ConstRef =
    std::reference_wrapper<std::add_const_t<std::remove_reference_t<T>>>;

/// `MutRef` is an always-mutable `Ref`
template <typename T>
using MutRef =
    std::reference_wrapper<std::remove_const_t<std::remove_reference_t<T>>>;

#if defined(__cpp_concepts)
#if __cpp_concepts >= 201907L
template <typename T, typename Base>
concept Impl = std::is_base_of<Base, T>::value;
#endif
#endif

namespace internal {

/// pointer hex formatting
///
/// 0xaabbccdd => 4 bytes (+ 8 chars)
///
/// 0xaa => 1 byte (+ 2 chars)
///
/// 0xaabb => 2 bytes (+ 4 chars)
///
/// we leave 2 extra bytes for tolerance
constexpr int kxPtrFmtSize = static_cast<int>((sizeof(void*) << 1) + 2 + 2);

// note that the sizes are also affected by the word size. i.e. int64_t and
// size_t will be 32-bit on some platforms, we thus can't allocate less memory
// than required on the stack, though it can be a bit much, such as int64_t on
// 32-bit platforms

/// 10 digits + 1 sign
constexpr int kI32FmtSize = 11;
/// 10 digits
constexpr int kU32FmtSize = 10;
/// 5 digits + 1 sign
constexpr int kI16FmtSize = 6;
/// 5 digits
constexpr int kU16FmtSize = 5;
/// 3 digits + 1 sign
constexpr int kI8FmtSize = 4;
/// 3 digits
constexpr int kU8FmtSize = 3;

}  // namespace internal

STX_END_NAMESPACE
