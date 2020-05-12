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
#include <utility>

#include "stx/config.h"

namespace stx {
/// NOTE: these implementations will be replaced by standard versions in future
/// release, most toolchains don't implement these yet

template <typename Fn, typename... Args>
using invoke_result = typename std::invoke_result<Fn, Args...>::type;

template <typename T>
using remove_ref = std::remove_reference_t<T>;

template <typename T>
concept Const = std::is_const_v<T>;

template <typename T>
concept NonConst = !Const<T>;

// TODO(lamarrr): we actually need a movable
template <typename T>
concept Swappable = std::is_swappable_v<T>;

template <typename T>
concept default_constructible = std::is_default_constructible_v<T>;

template <typename T>
concept copy_constructible = std::is_copy_constructible_v<T>;

template <typename T, typename Cmp>
concept same_as = std::is_same_v<T, Cmp>;

template <typename Src, typename Dest>
concept convertible_to = std::is_convertible_v<Src, Dest>;

/// helps to guide against implicit conversions, especially against character
/// pointers, string, and string_view.
/// Pointers are dangerous!
/// a string or string_view does not always
/// mean a char *, nor char const *, but they have implicit conversions. It is
/// not sensible to allow implicit conversions here.
template <typename T, typename Cmp>
concept exact = std::is_same_v<T, Cmp>;

template <typename Fn, typename... Args>
concept invocable = std::is_invocable_v<Fn, Args...>;

template <typename L, typename R = L>
concept equality_comparable = requires(std::remove_reference_t<L> const& l,
                                       std::remove_reference_t<R> const& r) {
  { l == r }
  ->same_as<bool>;
  { l != r }
  ->same_as<bool>;
  { l == r }
  ->same_as<bool>;
  { l != r }
  ->same_as<bool>;
};

template <typename T>
concept RawReference = std::is_reference_v<T>;

template <typename T>
concept Pointer = std::is_pointer_v<T>;

template <typename T>
concept UnaryDerefableSmartPointer =
    requires(T t, typename T::element_type element_type) {
  { *t }
  ->RawReference;
};

template <typename T>
concept OutputPointer =
    Pointer<T> && !(std::is_const_v<std::remove_pointer_t<T>>);

// any type of pointer is an input pointer
template <typename T>
concept InputPointer = Pointer<T>;

// not all pointers are iterators!!!
// an iterator is not constant
template <typename T>
concept Iterator =
    requires(std::iterator_traits<T> traits,
             typename std::iterator_traits<T>::value_type value,
             typename std::iterator_traits<T>::reference reference,
             typename std::iterator_traits<T>::pointer pointer,
             typename std::iterator_traits<T>::difference_type difference,
             typename std::iterator_traits<T>::iterator_category category) {
  {traits};
}
&&(
    requires(T iter) {
      { iter++ }
      ->convertible_to<T>;
    } ||
    requires(T iter) {
      { iter-- }
      ->convertible_to<T>;
    });

template <typename T>
concept Derefable = Pointer<T> || Iterator<T> || UnaryDerefableSmartPointer<T>;

namespace internal {
template <Derefable T>
using DerefValue__ = std::remove_reference_t<decltype(*std::declval<T>())>;
};

template <typename T>
concept OutputIterator = Iterator<T>&& NonConst<internal::DerefValue__<T>>;

// any type of pointer is an input iterator
template <typename T>
concept InputIterator = Iterator<T>;

// any type of dereferencable is const dereferencable
template <typename T>
concept ConstDerefable = Derefable<T>;

template <typename T>
using Ref = std::reference_wrapper<T>;

template <NonConst T>
using MutRef = std::reference_wrapper<T>;

template <typename T>
using ConstRef = std::reference_wrapper<T const>;

template <typename T>
concept MutDerefable = Derefable<T> && (OutputPointer<T> || OutputIterator<T>);

template <Derefable T>
using Deref = std::reference_wrapper<internal::DerefValue__<T>>;

template <ConstDerefable T>
using ConstDeref = std::reference_wrapper<internal::DerefValue__<T> const>;

template <MutDerefable T>
using MutDeref = Deref<T>;

};  // namespace stx
