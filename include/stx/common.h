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

#ifndef STX_COMMON_H_
#define STX_COMMON_H_

#include <version>

#if (__cpp_concepts < 201907L) || !defined(__cpp_concepts)
#error "2019/07 version of concepts is not supported on this compiler"
#endif

#include <functional>
#include <iterator>
#include <type_traits>
#include <utility>

namespace stx {

template <typename Fn, typename... Args>
using invoke_result = typename std::invoke_result<Fn, Args...>::type;

template <typename T>
concept Const = std::is_const_v<T>;

template <typename T>
concept NonConst = !Const<T>;

template <typename T>
using Ref = std::reference_wrapper<T>;

template <NonConst T>
using MutRef = std::reference_wrapper<T>;

template <typename T>
using ConstRef = std::reference_wrapper<T const>;

template <typename T>
concept Swappable = std::is_swappable_v<T>;

template <typename T>
concept DefaultConstructible = std::is_default_constructible_v<T>;

template <typename T>
concept CopyConstructible = std::is_copy_constructible_v<T>;

template <typename T, typename Cmp>
concept same_as = std::is_same_v<T, Cmp>;

template <typename T, typename Cmp>
concept Same = std::is_same_v<T, Cmp>;

template <typename Fn, typename Arg>
concept OneArgInvocable = std::is_invocable_v<Fn, Arg>;

template <typename Fn>
concept NoArgInvocable = std::is_invocable_v<Fn>;

template <typename Result, typename Fn, typename... Args>
concept InvokeResult =
    std::is_same_v<std::invoke_result_t<Fn, Args...>, Result>;

template <typename R, typename L = R>
concept EqualityComparable = requires(R const& r, L const& l) {
  { l == r }
  ->same_as<bool>;
};

template <typename T>
void dummy__(T);

// implicit
template <typename T, typename Arg>
concept ImplicitlyConstructibleWith = requires(Arg arg) {
  {dummy__<T>(std::forward<Arg>(arg))};
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
    Pointer<T> && !(std::is_const_v<std::remove_pointer_t<T> >);

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
};

template <typename T>
concept Derefable = Pointer<T> || Iterator<T> || UnaryDerefableSmartPointer<T>;

template <Derefable T>
using DerefValue__ = std::remove_reference_t<decltype(*std::declval<T>())>;

template <typename T>
concept OutputIterator = Iterator<T>&& NonConst<DerefValue__<T> >;

// any type of pointer is an input iterator
template <typename T>
concept InputIterator = Iterator<T>;

// any type of dereferencable is const dereferencable
template <typename T>
concept ConstDerefable = Derefable<T>;

template <typename T>
concept MutDerefable = Derefable<T> && (OutputPointer<T> || OutputIterator<T>);

template <Derefable T>
using Deref = std::reference_wrapper<DerefValue__<T> >;

template <ConstDerefable T>
using ConstDeref = std::reference_wrapper<DerefValue__<T> const>;

template <MutDerefable T>
using MutDeref = Deref<T>;

};      // namespace stx
#endif  // STX_COMMON_H_
