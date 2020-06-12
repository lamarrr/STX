/**
 * @file option_result.h
 * @author Basit Ayantunde <rlamarrr@gmail.com>
 * @date 2020-04-16
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

#include <utility>

#include "stx/internal/panic_helpers.h"

// Why so long? Option and Result depend on each other. I don't know of a
// way to break the cyclic dependency, primarily because they are templated
//
// Lifetime notes:
// - Every change of state must be followed by a destruction (and construction
// if it has a non-null variant)
// - The object must be destroyed immediately after the contained value is moved
// from it.
//
// Notes:
// - Result is an object-forwarding type. It is unique to an
// interaction. It functions like a std::unique_ptr, as it doesn't allow
// implicitly copying its data content. Unless explicitly stated via the
// .clone() method
// - We strive to make lifetime paths as visible and predictable as
// possible
// - We also try to prevent you from shooting yourself in the foot, especially
// with references and implicit copies
// - Many are marked constexpr but won't work in a constexpr context in C++ 17
// mode. They work in C++ 20 and we don't want macros for that. To add, many
// compilers labelled with 'C++20' might not have non-trivial constexpr
// destructors.

//! @file
//!
//! to run tests, use:
//!
//! ``` cpp
//!
//! #include <iostream>
//! #include <string>
//! #include <string_view>
//!
//!
//! using std::move, std::string, std::string_view;
//! using namespace std::literals; // makes '"Hello"s' give std::string
//!                                // and '"Hello"sv' give std::string_view
//!
//! ```

STX_BEGIN_NAMESPACE

template <typename T>
struct Some;

struct NoneType;

template <typename T>
struct Option;

template <typename T>
struct Ok;

template <typename E>
struct Err;

template <typename T, typename E>
struct Result;

//! value-variant Type for `Option<T>` representing no-value
//!
//! # Constexpr ?
//!
//! C++ 17 and above
//!
struct [[nodiscard]] NoneType {
  constexpr NoneType() noexcept = default;
  constexpr NoneType(NoneType const&) noexcept = default;
  constexpr NoneType(NoneType &&) noexcept = default;
  constexpr NoneType& operator=(NoneType const&) noexcept = default;
  constexpr NoneType& operator=(NoneType&&) noexcept = default;

  STX_CXX20_DESTRUCTOR_CONSTEXPR ~NoneType() noexcept = default;

  [[nodiscard]] constexpr bool operator==(NoneType const&) const noexcept {
    return true;
  }

  [[nodiscard]] constexpr bool operator!=(NoneType const&) const noexcept {
    return false;
  }

  template <typename T>
  [[nodiscard]] constexpr bool operator==(Some<T> const&) const noexcept {
    return false;
  }

  template <typename T>
  [[nodiscard]] constexpr bool operator!=(Some<T> const&) const noexcept {
    return true;
  }
};

/// value-variant for `Option<T>` representing no-value
constexpr NoneType const None{};

//! value-variant for `Option<T>` wrapping the contained value
//!
//! # Usage
//!
//! Note that `Some` is only a value-forwarding type. It doesn't make copies of
//! its constructor arguments and only accepts r-values.
//!
//! What does this mean?
//!
//! For example, You can:
//!
//! ```cpp
//! Option a = Some(vector{1, 2, 3, 4});
//! ```
//! You can't:
//!
//! ```cpp
//! vector<int> x {1, 2, 3, 4};
//! Option a = Some(x);
//! ```
//! But, to explicitly make `a` take ownership, you will:
//!
//! ```cpp
//! vector<int> x {1, 2, 3, 4};
//! Option a = Some(std::move(x));
//! ```
//!
//! # Constexpr ?
//!
//! C++ 17 and above
//!
template <typename T>
struct [[nodiscard]] Some {
  static_assert(movable<T>, "Value type 'T' for 'Option<T>' must be movable");
  static_assert(
      !is_reference<T>,
      "Cannot use a reference for value type 'T' of 'Option<T>' , To prevent "
      "subtleties use "
      "type wrappers like std::reference_wrapper (stx::Ref) or any of the "
      "`stx::ConstRef` or `stx::MutRef` specialized aliases instead");

  using value_type = T;

  /// a `Some<T>` can only be constructed with an r-value of type `T`
  explicit constexpr Some(T && value) : value_(std::forward<T&&>(value)) {}

  constexpr Some(Some && rhs) = default;
  constexpr Some& operator=(Some&& rhs) = default;
  constexpr Some(Some const&) = default;
  constexpr Some& operator=(Some const&) = default;

  STX_CXX20_DESTRUCTOR_CONSTEXPR ~Some() = default;

  [[nodiscard]] constexpr T const& value() const& noexcept { return value_; }
  [[nodiscard]] constexpr T& value()& noexcept { return value_; }
  [[nodiscard]] constexpr T const value() const&& { return std::move(value_); }
  [[nodiscard]] constexpr T value()&& { return std::move(value_); }

  template <typename U>
  [[nodiscard]] constexpr bool operator==(Some<U> const& cmp) const {
    static_assert(equality_comparable<T, U>);
    return value() == cmp.value();
  }

  template <typename U>
  [[nodiscard]] constexpr bool operator!=(Some<U> const& cmp) const {
    static_assert(equality_comparable<T, U>);
    return value() != cmp.value();
  }

  [[nodiscard]] constexpr bool operator==(NoneType const&) const noexcept {
    return false;
  }

  [[nodiscard]] constexpr bool operator!=(NoneType const&) const noexcept {
    return true;
  }

 private:
  T value_;

  template <typename Tp>
  friend struct Option;
};

//! value-variant for `Result<T, E>` wrapping the contained successful value of
//! type `T`
//!
//! Note that `Ok` is only a value-forwarding type.
//! An `Ok<T>` can only be constructed with an r-value. What does this mean?
//!
//! For example, You can:
//!
//! ```cpp
//! Result<vector<int>, int> a = Ok(vector{1, 2, 3, 4});
//! ```
//! You can't:
//!
//! ```cpp
//! vector<int> x {1, 2, 3, 4};
//! Result<vector<int>, int> a = Ok(x);
//! ```
//! But, to explicitly make `a` take ownership, you will:
//!
//! ```cpp
//! vector<int> x {1, 2, 3, 4};
//! Result<vector<int>, int> a = Ok(std::move(x));
//! ```
//!
//! # Constexpr ?
//!
//! C++ 17 and above
//!
template <typename T>
struct [[nodiscard]] Ok {
  static_assert(movable<T>, "Value type 'T' for 'Ok<T>' must be movable");
  static_assert(
      !is_reference<T>,
      "Cannot use a reference for value type 'T' of 'Ok<T>' , To prevent "
      "subtleties use "
      "type wrappers like std::reference_wrapper (stx::Ref) or any of the "
      "`stx::ConstRef` or `stx::MutRef` specialized aliases instead");

  using value_type = T;

  /// an `Ok<T>` can only be constructed with an r-value of type `T`
  explicit constexpr Ok(T && value) : value_(std::forward<T&&>(value)) {}

  constexpr Ok(Ok && rhs) = default;
  constexpr Ok& operator=(Ok&& rhs) = default;
  constexpr Ok(Ok const&) = default;
  constexpr Ok& operator=(Ok const&) = default;

  STX_CXX20_DESTRUCTOR_CONSTEXPR ~Ok() = default;

  template <typename U>
  [[nodiscard]] constexpr bool operator==(Ok<U> const& cmp) const {
    static_assert(equality_comparable<T, U>);
    return value() == cmp.value();
  }

  template <typename U>
  [[nodiscard]] constexpr bool operator!=(Ok<U> const& cmp) const {
    static_assert(equality_comparable<T, U>);
    return value() != cmp.value();
  }

  template <typename U>
  [[nodiscard]] constexpr bool operator==(Err<U> const&) const noexcept {
    return false;
  }

  template <typename U>
  [[nodiscard]] constexpr bool operator!=(Err<U> const&) const noexcept {
    return true;
  }

  [[nodiscard]] constexpr T const& value() const& noexcept { return value_; }
  [[nodiscard]] constexpr T& value()& noexcept { return value_; }
  [[nodiscard]] constexpr T const value() const&& { return std::move(value_); }
  [[nodiscard]] constexpr T value()&& { return std::move(value_); }

 private:
  T value_;

  template <typename Tp, typename Er>
  friend struct Result;
};

//! error-value variant for `Result<T, E>` wrapping the contained error value of
//! type `E`
//!
//! Note that `Err` is only a value-forwarding type.
//! An `Err<E>` can only be constructed with an r-value of type `E`. What does
//! this mean?
//!
//! For example, You can:
//!
//! ```cpp
//! Result<int, vector<int>> a = Err(vector{1, 2, 3, 4});
//! ```
//! You can't:
//!
//! ```cpp
//! vector<int> x {1, 2, 3, 4};
//! Result<int, vector<int>> a = Err(x);
//! ```
//! But, to explicitly make `a` take ownership, you will:
//!
//! ```cpp
//! vector<int> x {1, 2, 3, 4};
//! Result<int, vector<int>> a = Err(std::move(x));
//! ```
//!
//! # Constexpr ?
//!
//! C++ 17 and above
//!
template <typename E>
struct [[nodiscard]] Err {
  static_assert(movable<E>, "Error type 'E' for 'Err<E>' must be movable");
  static_assert(
      !is_reference<E>,
      "Cannot use a reference for error type 'E' of 'Err<E>' , To prevent "
      "subtleties use "
      "type wrappers like std::reference_wrapper (stx::Ref) or any of the "
      "`stx::ConstRef` or `stx::MutRef` specialized aliases instead");

  using value_type = E;

  /// an `Err<E>` can only be constructed with an r-value of type `E`
  explicit constexpr Err(E && value) : value_(std::forward<E&&>(value)) {}

  constexpr Err(Err && rhs) = default;
  constexpr Err& operator=(Err&& rhs) = default;
  constexpr Err(Err const&) = default;
  constexpr Err& operator=(Err const&) = default;

  STX_CXX20_DESTRUCTOR_CONSTEXPR ~Err() = default;

  template <typename F>
  [[nodiscard]] constexpr bool operator==(Err<F> const& cmp) const {
    static_assert(equality_comparable<E, F>);
    return value() == cmp.value();
  }

  template <typename F>
  [[nodiscard]] constexpr bool operator!=(Err<F> const& cmp) const {
    static_assert(equality_comparable<E, F>);
    return value() != cmp.value();
  }

  template <typename F>
  [[nodiscard]] constexpr bool operator==(Ok<F> const&) const noexcept {
    return false;
  }

  template <typename F>
  [[nodiscard]] constexpr bool operator!=(Ok<F> const&) const noexcept {
    return true;
  }

  [[nodiscard]] constexpr E const& value() const& noexcept { return value_; }
  [[nodiscard]] constexpr E& value()& noexcept { return value_; }
  [[nodiscard]] constexpr E const value() const&& { return std::move(value_); }
  [[nodiscard]] constexpr E value()&& { return std::move(value_); }

 private:
  E value_;

  template <typename Tp, typename Er>
  friend struct Result;
};

// JUST LOOK AWAY

namespace internal {
namespace option {
// constructs an r-value reference to the option's value directly, without
// checking if it is in the `Some` or `None` state. This is totally unsafe and
// user-end code should **never** use this
template <typename Tp>
inline Tp&& unsafe_value_move(Option<Tp>&);

}  // namespace option
}  // namespace internal

//! Optional values.
//!
//! Type `Option` represents an optional value: every `Option`
//! is either `Some` and contains a value, or `None`, and
//! does not.
//! They have a number of uses:
//!
//! * Initial values
//! * Return values for functions that are not defined over their entire input
//! range (partial functions)
//! * Return value for otherwise reporting simple errors, where `None` is
//! returned on error
//! * Optional struct fields
//! * Struct fields that can be loaned or "taken"
//! * Optional function arguments
//! * Nullable pointers
//! * Swapping things out of difficult situations
//!
//! `Option`'s are commonly paired with pattern matching to query the
//! presence of a value and take action, always accounting for the `None`s
//! case.
//!
//! ```
//! auto divide = [](double numerator, double denominator) -> Option<double> {
//!   if (denominator == 0.0) {
//!     return None;
//!   } else {
//!     return Some(numerator / denominator);
//!   }
//! };
//!
//! // The return value of the function is an option
//! auto result = divide(2.0, 3.0);
//! result.match([](double& value) { std::cout << value << std::endl; },
//!              []() { std::cout << "has no value" << std::endl; });
//! ```
//!
//!
//! # Constexpr ?
//!
//! C++ 20 and above
//!
template <typename T>
struct [[nodiscard]] Option {
 public:
  using value_type = T;

  static_assert(movable<T>, "Value type 'T' for 'Option<T>' must be movable");
  static_assert(
      !is_reference<T>,
      "Cannot use a reference for value type 'T' of 'Option<T>' , To prevent "
      "subtleties use "
      "type wrappers like std::reference_wrapper (stx::Ref) or any of the "
      "`stx::ConstRef` or `stx::MutRef` specialized aliases instead");

  constexpr Option() noexcept : is_none_(true) {}

  constexpr Option(Some<T> && some)
      : storage_value_(std::move(some.value_)), is_none_(false) {}

  constexpr Option(Some<T> const& some)
      : storage_value_(some.value()), is_none_(false) {
    static_assert(copy_constructible<T>);
  }

  constexpr Option(NoneType const&) noexcept : is_none_(true) {}

  // constexpr?
  // placement-new!
  // we can't make this constexpr as of C++ 20
  Option(Option && rhs) : is_none_(rhs.is_none_) {
    if (rhs.is_some()) {
      new (&storage_value_) T(std::move(rhs.storage_value_));
    }
  }

  Option& operator=(Option&& rhs) {
    // contained object is destroyed as appropriate in the parent scope
    if (is_some() && rhs.is_some()) {
      std::swap(storage_value_, rhs.storage_value_);
    } else if (is_some() && rhs.is_none()) {
      // we let the ref'd `rhs` destroy the object instead
      new (&rhs.storage_value_) T(std::move(storage_value_));
      storage_value_.~T();
      is_none_ = true;
      rhs.is_none_ = false;
    } else if (is_none() && rhs.is_some()) {
      new (&storage_value_) T(std::move(rhs.storage_value_));
      rhs.storage_value_.~T();
      rhs.is_none_ = true;
      is_none_ = false;
    }

    return *this;
  }

  Option(Option const& rhs) : is_none_(rhs.is_none_) {
    static_assert(copy_constructible<T>);
    if (rhs.is_some()) {
      new (&storage_value_) T(rhs.storage_value_);
    }
  }

  Option& operator=(Option const& rhs) {
    static_assert(copy_constructible<T>);

    if (is_some() && rhs.is_some()) {
      storage_value_ = rhs.storage_value_;
    } else if (is_some() && rhs.is_none()) {
      storage_value_.~T();
      is_none_ = true;
    } else if (is_none() && rhs.is_some()) {
      new (&storage_value_) T(rhs.storage_value_);
      is_none_ = false;
    }

    return *this;
  }

  STX_CXX20_DESTRUCTOR_CONSTEXPR ~Option() noexcept {
    if (is_some()) {
      storage_value_.~T();
    }
  }

  template <typename U>
  [[nodiscard]] constexpr bool operator==(Option<U> const& cmp) const {
    static_assert(equality_comparable<T, U>);
    if (is_some() && cmp.is_some()) {
      return value_cref_() == cmp.value_cref_();
    } else if (is_none() && cmp.is_none()) {
      return true;
    } else {
      return false;
    }
  }

  template <typename U>
  [[nodiscard]] constexpr bool operator!=(Option<U> const& cmp) const {
    static_assert(equality_comparable<T, U>);
    if (is_some() && cmp.is_some()) {
      return value_cref_() != cmp.value_cref_();
    } else if (is_none() && cmp.is_none()) {
      return false;
    } else {
      return true;
    }
  }

  template <typename U>
  [[nodiscard]] constexpr bool operator==(Some<U> const& cmp) const {
    static_assert(equality_comparable<T, U>);
    if (is_some()) {
      return value_cref_() == cmp.value();
    } else {
      return false;
    }
  }

  template <typename U>
  [[nodiscard]] constexpr bool operator!=(Some<U> const& cmp) const {
    static_assert(equality_comparable<T, U>);
    if (is_some()) {
      return value_cref_() != cmp.value();
    } else {
      return true;
    }
  }

  [[nodiscard]] constexpr bool operator==(NoneType const&) const noexcept {
    return is_none();
  }

  [[nodiscard]] constexpr bool operator!=(NoneType const&) const noexcept {
    return is_some();
  }

  /// Returns `true` if this Option is a `Some` value.
  ///
  /// # Examples
  ///
  /// Basic usage:
  ///
  /// ``` cpp
  /// Option x = Some(2);
  /// ASSERT_TRUE(x.is_some());
  ///
  /// Option<int> y = None;
  /// ASSERT_FALSE(y.is_some());
  /// ```
  ///
  [[nodiscard]] constexpr bool is_some() const noexcept { return !is_none(); }

  /// Returns `true` if the option is a `None` value.
  ///
  /// # Examples
  ///
  /// Basic usage:
  ///
  /// ``` cpp
  /// Option x = Some(2);
  /// ASSERT_FALSE(x.is_none());
  ///
  /// Option<int> y = None;
  /// ASSERT_TRUE(y.is_none());
  /// ```
  [[nodiscard]] constexpr bool is_none() const noexcept { return is_none_; }

  [[nodiscard]] operator bool() const noexcept { return is_some(); }

  /// Returns `true` if the option is a `Some` value containing the given
  /// value.
  ///
  /// # Examples
  ///
  /// Basic usage:
  ///
  /// ``` cpp
  /// Option x = Some(2);
  /// ASSERT_TRUE(x.contains(2));
  ///
  /// Option y = Some(3);
  /// ASSERT_FALSE(y.contains(2));
  ///
  /// Option<int> z = None;
  /// ASSERT_FALSE(z.contains(2));
  /// ```
  template <typename CmpType>
  [[nodiscard]] constexpr bool contains(CmpType const& cmp) const {
    static_assert(equality_comparable<T, CmpType>);
    if (is_some()) {
      return value_cref_() == cmp;
    } else {
      return false;
    }
  }

  /// Returns the value of evaluating the `predicate` on the contained value if
  /// the `Option` is a `Some`, else returns `false`.
  ///
  /// # Examples
  ///
  /// Basic usage:
  ///
  /// ``` cpp
  /// Option x = Some(2);
  /// auto even = [](auto x) { return x == 2; };
  /// ASSERT_TRUE(x.exists(even));
  ///
  /// ```
  template <typename UnaryPredicate>
  [[nodiscard]] constexpr bool exists(UnaryPredicate && predicate) const {
    static_assert(invocable<UnaryPredicate&&, T const&>);
    static_assert(convertible<invoke_result<UnaryPredicate&&, T const&>, bool>);

    if (is_some()) {
      return std::forward<UnaryPredicate&&>(predicate)(value_cref_());
    } else {
      return false;
    }
  }

  /// Returns an l-value reference to the contained value.
  /// Note that no copying occurs here.
  ///
  /// # Panics
  ///
  /// Panics if the value is a `None`
  ///
  /// # Examples
  ///
  /// Basic usage:
  ///
  /// ``` cpp
  /// auto x = make_some(9);
  /// int& y = x.value();
  /// y = 2;
  ///
  /// ASSERT_EQ(x, Some(2));
  /// ```
  [[nodiscard]] T& value()& noexcept {
    if (is_none()) internal::option::no_lref();
    return value_ref_();
  }

  /// Returns a const l-value reference to the contained value.
  /// Note that no copying occurs here.
  ///
  /// # Panics
  ///
  /// Panics if the value is a `None`
  ///
  /// # Examples
  ///
  /// Basic usage:
  ///
  /// ``` cpp
  /// auto const x = make_some(9);
  /// int const& y = x.value();
  ///
  /// ASSERT_EQ(y, 9);
  /// ```
  [[nodiscard]] T const& value() const& noexcept {
    if (is_none()) internal::option::no_lref();
    return value_cref_();
  }

  /// Use `unwrap()` instead
  [[deprecated("Use `unwrap()` instead")]] T value()&& = delete;
  /// Use `unwrap()` instead
  [[deprecated("Use `unwrap()` instead")]] T const value() const&& = delete;

  /// Converts from `Option<T> const&` or `Option<T> &` to
  /// `Option<ConstRef<T>>`.
  ///
  /// # NOTE
  /// `ConstRef<T>` is an alias for `std::reference_wrapper<T const>` and
  /// guides against reference-collapsing
  [[nodiscard]] constexpr auto as_cref() const& noexcept->Option<ConstRef<T>> {
    if (is_some()) {
      return Some<ConstRef<T>>(ConstRef<T>(value_cref_()));
    } else {
      return None;
    }
  }

  [[deprecated(
      "calling Option::as_cref() on an r-value, and therefore binding a "
      "reference to an object that is marked to be moved")]]  //
  [[nodiscard]] constexpr auto
  as_cref() const&& noexcept->Option<ConstRef<T>> = delete;

  /// Converts from `Option<T>` to `Option<MutRef<T>>`.
  ///
  /// # Examples
  ///
  /// Basic usage:
  ///
  /// ``` cpp
  /// auto mutate = [](Option<int>& r) {
  ///  r.as_ref().match([](Ref<int> ref) { ref.get() = 42; },
  ///                   []() { });
  /// };
  ///
  /// auto x = make_some(2);
  /// mutate(x);
  /// ASSERT_EQ(x, Some(42));
  ///
  /// auto y = make_none<int>();
  /// mutate(y);
  /// ASSERT_EQ(y, None);
  /// ```
  [[nodiscard]] constexpr auto as_ref()& noexcept->Option<MutRef<T>> {
    if (is_some()) {
      return Some<MutRef<T>>(MutRef<T>(value_ref_()));
    } else {
      return None;
    }
  }

  [[nodiscard]] constexpr auto as_ref() const& noexcept->Option<ConstRef<T>> {
    return as_cref();
  }

  [[deprecated(
      "calling Option::as_ref() on an r-value, and therefore binding a "
      "reference to an object that is marked to be moved")]]  //
  [[nodiscard]] constexpr auto
  as_ref()&& noexcept->Option<MutRef<T>> = delete;

  [[deprecated(
      "calling Option::as_ref() on an r-value, and therefore binding a "
      "reference to an object that is marked to be moved")]]  //
  [[nodiscard]] constexpr auto
  as_ref() const&& noexcept->Option<ConstRef<T>> = delete;

  /// Unwraps an option, yielding the content of a `Some`.
  ///
  /// # Panics
  ///
  /// Panics if the value is a `None` with a custom panic message provided by
  /// `msg`.
  ///
  ///
  /// # Examples
  ///
  /// Basic usage:
  ///
  /// ``` cpp
  /// Option x = Some("value"s);
  /// ASSERT_EQ(move(x).expect("the world is ending"), "value");
  ///
  /// Option<string> y = None;
  /// ASSERT_DEATH(move(y).expect("the world is ending")); // panics with
  ///                                                          // the world is
  ///                                                          // ending
  /// ```
  [[nodiscard]] auto expect(std::string_view const& msg)&&->T {
    if (is_some()) {
      return std::move(value_ref_());
    } else {
      internal::option::expect_value_failed(msg);
    }
  }

  /// Moves the value out of the `Option<T>` if it is in the variant state of
  /// `Some<T>`.
  ///
  /// In general, because this function may panic, its use is discouraged.
  /// Instead, prefer to use pattern matching and handle the `None`
  /// case explicitly.
  ///
  /// # Panics
  ///
  /// Panics if its value equals `None`.
  ///
  /// # Examples
  ///
  /// Basic usage:
  ///
  /// ``` cpp
  /// Option x = Some("air"s);
  /// ASSERT_EQ(move(x).unwrap(), "air");
  ///
  /// Option<string> y = None;
  /// ASSERT_DEATH(move(y).unwrap());
  /// ```
  [[nodiscard]] auto unwrap()&&->T {
    if (is_some()) {
      return std::move(value_ref_());
    } else {
      internal::option::no_value();
    }
  }

  /// Returns the contained value or an alternative: `alt`.
  ///
  /// Arguments passed to `unwrap_or` are eagerly evaluated; if you are passing
  /// the result of a function call, it is recommended to use `unwrap_or_else`,
  /// which is lazily evaluated.
  ///
  ///
  /// # Examples
  ///
  /// Basic usage:
  ///
  /// ``` cpp
  /// ASSERT_EQ(Option(Some("car"s)).unwrap_or("bike"), "car");
  /// ASSERT_EQ(make_none<string>().unwrap_or("bike"), "bike");
  /// ```
  [[nodiscard]] constexpr auto unwrap_or(T && alt)&&->T {
    if (is_some()) {
      return std::move(value_ref_());
    } else {
      return std::move(alt);
    }
  }

  /// Returns the contained value or computes it from a function.
  ///
  /// # Examples
  ///
  /// Basic usage:
  ///
  /// ``` cpp
  /// int k = 10;
  /// auto alt = [&k]() { return 2 * k; };
  ///
  /// ASSERT_EQ(make_some(4).unwrap_or_else(alt), 4);
  /// ASSERT_EQ(make_none<int>().unwrap_or_else(alt), 20);
  /// ```
  template <typename Fn>
  [[nodiscard]] constexpr auto unwrap_or_else(Fn && op)&&->T {
    static_assert(invocable<Fn&&>);
    if (is_some()) {
      return std::move(value_ref_());
    } else {
      return std::forward<Fn&&>(op)();
    }
  }

  /// Maps an `Option<T>` to `Option<U>` by applying a function to a contained
  /// value and therefore, consuming/moving the contained value.
  ///
  /// # Examples
  ///
  /// Basic usage:
  ///
  /// Converts an `Option<string>` into an `Option<size_t>`,
  /// consuming the original:
  ///
  ///
  /// ```
  /// Option maybe_string = Some("Hello, World!"s);
  /// // `Option::map` will only work on Option as an r-value and assumes the
  /// //  object in it is about to be moved
  /// auto maybe_len = move(maybe_string).map([](auto s){ return s.size();
  /// });
  /// // maybe_string is invalid and should not be used from here since we
  /// // `std::move`-d from it
  ///
  /// ASSERT_EQ(maybe_len, Some<size_t>(13));
  /// ```
  template <typename Fn>
  [[nodiscard]] constexpr auto map(Fn &&
                                   op)&&->Option<invoke_result<Fn&&, T&&>> {
    static_assert(invocable<Fn&&, T&&>);
    if (is_some()) {
      return Some<invoke_result<Fn&&, T&&>>(
          std::forward<Fn&&>(op)(std::move(value_ref_())));
    } else {
      return None;
    }
  }

  /// Applies a function to the contained value (if any),
  /// or returns the provided alternative (if not).
  ///
  /// # Examples
  ///
  /// Basic usage:
  ///
  /// ``` cpp
  /// Option x = Some("foo"s);
  /// auto alt_fn = [](auto s) { return s.size(); };
  /// ASSERT_EQ(move(x).map_or(alt_fn, 42UL), 3UL);
  ///
  /// Option<string> y = None;
  /// ASSERT_EQ(move(y).map_or(alt_fn, 42UL), 42UL);
  /// ```
  template <typename Fn, typename A>
  [[nodiscard]] constexpr auto map_or(Fn && op,
                                      A && alt)&&->invoke_result<Fn&&, T&&> {
    static_assert(invocable<Fn&&, T&&>);
    if (is_some()) {
      return std::forward<Fn&&>(op)(std::move(value_ref_()));
    } else {
      return std::forward<A&&>(alt);
    }
  }

  /// Applies a function to the contained value (if any),
  /// or computes a default (if not).
  ///
  /// # Examples
  ///
  /// Basic usage:
  ///
  /// ``` cpp
  /// size_t k = 21;
  /// auto map_fn = [] (auto s) { return s.size(); };
  /// auto alt_fn = [&k] () { return 2UL * k; };
  ///
  /// Option x = Some("foo"s);
  /// ASSERT_EQ(move(x).map_or_else(map_fn, alt_fn), 3);
  ///
  /// Option<string> y = None;
  /// ASSERT_EQ(move(y).map_or_else(map_fn, alt_fn), 42);
  /// ```
  template <typename Fn, typename AltFn>
  [[nodiscard]] constexpr auto map_or_else(
      Fn && op, AltFn && alt)&&->invoke_result<Fn&&, T&&> {
    static_assert(invocable<Fn&&, T&&>);
    static_assert(invocable<AltFn&&>);

    if (is_some()) {
      return std::forward<Fn&&>(op)(std::move(value_ref_()));
    } else {
      return std::forward<AltFn&&>(alt)();
    }
  }

  /// Transforms the `Option<T>` into a `Result<T, E>`, mapping `Some<T>` to
  /// `Ok<T>` and `None` to `Err<E>`.
  ///
  /// Arguments passed to `ok_or` are eagerly evaluated; if you are passing the
  /// result of a function call, it is recommended to use `ok_or_else`, which is
  /// lazily evaluated.
  ///
  /// # Examples
  ///
  /// Basic usage:
  ///
  /// ``` cpp
  /// Option x = Some("foo"s);
  /// ASSERT_EQ(move(x).ok_or(0), Ok("foo"s));
  ///
  /// Option<string> y = None;
  /// ASSERT_EQ(move(y).ok_or(0), Err(0));
  /// ```
  // copies the argument if not an r-value
  template <typename E>
  [[nodiscard]] constexpr auto ok_or(E error)&&->Result<T, E> {
    if (is_some()) {
      return Ok<T>(std::move(value_ref_()));
    } else {
      // tries to copy if it is an l-value ref and moves if it is an r-value
      return Err<E>(std::forward<E>(error));
    }
  }

  /// Transforms the `Option<T>` into a `Result<T, E>`, mapping `Some<T>` to
  /// `Ok<T>` and `None` to `Err(op())`.
  ///
  /// # Examples
  ///
  /// Basic usage:
  ///
  /// ``` cpp
  /// auto else_fn = [] () { return 0; };
  ///
  /// Option x = Some("foo"s);
  /// ASSERT_EQ(move(x).ok_or_else(else_fn), Ok("foo"s));
  ///
  /// Option<string> y = None;
  /// ASSERT_EQ(move(y).ok_or_else(else_fn), Err(0));
  /// ```
  // can return reference but the user will get the error
  template <typename Fn>
  [[nodiscard]] constexpr auto ok_or_else(
      Fn && op)&&->Result<T, invoke_result<Fn&&>> {
    static_assert(invocable<Fn&&>);
    if (is_some()) {
      return Ok<T>(std::move(value_ref_()));
    } else {
      return Err<invoke_result<Fn&&>>(std::forward<Fn&&>(op)());
    }
  }

  /// Returns `None` if the option is `None`, otherwise returns `cmp`.
  ///
  /// # Examples
  ///
  /// Basic usage:
  ///
  /// ``` cpp
  /// Option a = Some(2);
  /// Option<string> b = None;
  /// ASSERT_EQ(move(a).AND(move(b)), None);
  ///
  /// Option<int> c = None;
  /// Option d = Some("foo"s);
  /// ASSERT_EQ(move(c).AND(move(d)), None);
  ///
  /// Option e = Some(2);
  /// Option f = Some("foo"s);
  /// ASSERT_EQ(move(e).AND(move(f)), Some("foo"s));
  ///
  /// Option<int> g = None;
  /// Option<string> h = None;
  /// ASSERT_EQ(move(g).AND(move(h)), None);
  /// ```
  // won't compile if a normal reference is passed since it is not copyable
  // if an rvalue, will pass. We are not forwarding refences here.
  // a requirement here is for it to be constructible with a None
  template <typename U>  //
  [[nodiscard]] constexpr auto AND(Option<U> && cmp)&&->Option<U> {
    if (is_some()) {
      return std::forward<Option<U>&&>(cmp);
    } else {
      return None;
    }
  }

  /// Returns `None` if the option is `None`, otherwise calls `op` with the
  /// wrapped value and returns the result.
  ///
  /// Some languages call this operation flatmap.
  ///
  /// # Examples
  ///
  /// Basic usage:
  ///
  /// ``` cpp
  /// auto sq = [] (auto x) -> Option<int> { return Some(x * x); };
  /// auto nope = [] (auto) -> Option<int> { return None; };
  ///
  /// ASSERT_EQ(make_some(2).and_then(sq).and_then(sq), Some(16));
  /// ASSERT_EQ(make_some(2).and_then(sq).and_then(nope), None);
  /// ASSERT_EQ(make_some(2).and_then(nope).and_then(sq), None);
  /// ASSERT_EQ(make_none<int>().and_then(sq).and_then(sq), None);
  /// ```
  template <typename Fn>
  [[nodiscard]] constexpr auto and_then(Fn && op)&&->invoke_result<Fn&&, T&&> {
    static_assert(invocable<Fn&&, T&&>);
    if (is_some()) {
      return std::forward<Fn&&>(op)(std::move(value_ref_()));
    } else {
      return None;
    }
  }

  /// Returns `None` if the option is `None`, otherwise calls `predicate`
  /// with the wrapped value and returns:
  ///
  /// - `Some<T>` if `predicate` returns `true` on invocation on the value.
  /// - `None` if `predicate` returns `false` on invocation on the value.
  ///
  /// `filter()` lets you decide which elements to keep.
  ///
  /// # Examples
  ///
  /// Basic usage:
  ///
  /// ``` cpp
  /// auto is_even = [](int n) -> bool { return n % 2 == 0; };
  ///
  /// ASSERT_EQ(make_none<int>().filter(is_even), None);
  /// ASSERT_EQ(make_some(3).filter(is_even), None);
  /// ASSERT_EQ(make_some(4).filter(is_even), Some(4));
  /// ```
  template <typename UnaryPredicate>
  [[nodiscard]] constexpr auto filter(UnaryPredicate && predicate)&&->Option {
    static_assert(invocable<UnaryPredicate&&, T const&>);
    static_assert(convertible<invoke_result<UnaryPredicate&&, T const&>, bool>);

    if (is_some() && std::forward<UnaryPredicate&&>(predicate)(value_cref_())) {
      return std::move(*this);
    } else {
      return None;
    }
  }

  /// Returns `None` if the option is `None`, otherwise calls `predicate`
  /// with the wrapped value and returns:
  ///
  /// - `Some<T>` if `predicate` returns `false` on invocation on the value.
  /// - `None` if `predicate` returns `true` on invocation on the value.
  ///
  /// `filter_not()` lets you decide which elements to keep.
  ///
  /// # Examples
  ///
  /// Basic usage:
  ///
  /// ``` cpp
  /// auto is_even = [](int n) -> bool { return n % 2 == 0; };
  ///
  /// ASSERT_EQ(make_none<int>().filter_not(is_even), None);
  /// ASSERT_EQ(make_some(3).filter_not(is_even), Some(3));
  /// ASSERT_EQ(make_some(4).filter_not(is_even), None);
  /// ```
  template <typename UnaryPredicate>
  [[nodiscard]] constexpr auto filter_not(UnaryPredicate &&
                                          predicate)&&->Option {
    static_assert(invocable<UnaryPredicate&&, T const&>);
    static_assert(convertible<invoke_result<UnaryPredicate&&, T const&>, bool>);

    if (is_some() &&
        !std::forward<UnaryPredicate&&>(predicate)(value_cref_())) {
      return std::move(*this);
    } else {
      return None;
    }
  }

  /// Returns the option if it contains a value, otherwise returns `alt`.
  ///
  /// Arguments passed to `OR` are eagerly evaluated; if you are passing the
  /// result of a function call, it is recommended to use `or_else`, which is
  /// lazily evaluated.
  ///
  ///
  /// # Examples
  ///
  /// Basic usage:
  ///
  /// ``` cpp
  /// Option a = Some(2);
  /// Option<int> b = None;
  /// ASSERT_EQ(move(a).OR(move(b)), Some(2));
  ///
  /// Option<int> c = None;
  /// Option d = Some(100);
  /// ASSERT_EQ(move(c).OR(move(d)), Some(100));
  ///
  /// Option e = Some(2);
  /// Option f = Some(100);
  /// ASSERT_EQ(move(e).OR(move(f)), Some(2));
  ///
  /// Option<int> g = None;
  /// Option<int> h = None;
  /// ASSERT_EQ(move(g).OR(move(h)), None);
  /// ```
  [[nodiscard]] constexpr auto OR(Option && alt)&&->Option {
    if (is_some()) {
      return std::move(*this);
    } else {
      return std::move(alt);
    }
  }

  /// Returns the option if it contains a value, otherwise calls `f` and
  /// returns the result.
  ///
  /// # Examples
  ///
  /// Basic usage:
  ///
  /// ``` cpp
  /// auto nobody = []() -> Option<string> { return None; };
  /// auto vikings = []() -> Option<string> { return Some("vikings"s); };
  ///
  /// ASSERT_EQ(Option(Some("barbarians"s)).or_else(vikings),
  /// Some("barbarians"s));
  /// ASSERT_EQ(make_none<string>().or_else(vikings), Some("vikings"s));
  /// ASSERT_EQ(make_none<string>().or_else(nobody), None);
  /// ```
  template <typename Fn>
  [[nodiscard]] constexpr auto or_else(Fn && op)&&->Option {
    static_assert(invocable<Fn&&>);
    if (is_some()) {
      return std::move(*this);
    } else {
      return std::forward<Fn&&>(op)();
    }
  }

  /// Returns whichever one of this object or `alt` is a `Some<T>` variant,
  /// otherwise returns `None` if neither or both are a `Some<T>` variant.
  ///
  ///
  /// # Examples
  ///
  /// Basic usage:
  ///
  /// ``` cpp
  /// Option a = Some(2);
  /// Option<int> b = None;
  /// ASSERT_EQ(move(a).XOR(move(b)), Some(2));
  ///
  /// Option<int> c = None;
  /// Option d = Some(2);
  /// ASSERT_EQ(move(c).XOR(move(d)), Some(2));
  ///
  /// Option e = Some(2);
  /// Option f = Some(2);
  /// ASSERT_EQ(move(e).XOR(move(f)), None);
  ///
  /// Option<int> g = None;
  /// Option<int> h = None;
  /// ASSERT_EQ(move(g).XOR(move(h)), None);
  /// ```
  [[nodiscard]] constexpr auto XOR(Option && alt)&&->Option {
    if (is_some() && alt.is_none()) {
      return std::move(*this);
    } else if (is_none() && alt.is_some()) {
      return std::move(alt);
    } else {
      return None;
    }
  }

  /// Takes the value out of the option, leaving a `None` in its place.
  ///
  ///
  /// # Examples
  ///
  /// Basic usage:
  ///
  /// ``` cpp
  /// Option a = Some(2);
  /// auto b = a.take();
  /// ASSERT_EQ(a, None);
  /// ASSERT_EQ(b, Some(2));
  ///
  /// Option<int> c  = None;
  /// auto d = c.take();
  /// ASSERT_EQ(c, None);
  /// ASSERT_EQ(d, None);
  /// ```
  [[nodiscard]] constexpr auto take()->Option {
    if (is_some()) {
      auto some = Some<T>(std::move(value_ref_()));
      value_ref_().~T();
      is_none_ = true;
      return some;
    } else {
      return None;
    }
  }

  /// Replaces the actual value in the option by the value given in parameter,
  /// returning the old value if present,
  /// leaving a `Some` in its place without deinitializing either one.
  ///
  ///
  /// # Examples
  ///
  /// Basic usage:
  ///
  /// ``` cpp
  /// Option x = Some(2);
  /// auto old_x = x.replace(5);
  /// ASSERT_EQ(x, Some(5));
  /// ASSERT_EQ(old_x, Some(2));
  ///
  /// Option<int> y = None;
  /// auto old_y = y.replace(3);
  /// ASSERT_EQ(y, Some(3));
  /// ASSERT_EQ(old_y, None);
  /// ```
  [[nodiscard]] auto replace(T && replacement)->Option {
    if (is_some()) {
      std::swap(replacement, value_ref_());
      return Some<T>(std::move(replacement));
    } else {
      new (&storage_value_) T(std::forward<T&&>(replacement));
      is_none_ = false;
      return None;
    }
  }

  /// Replaces the actual value in the option by the value given in parameter,
  /// returning the old value if present,
  /// leaving a `Some` in its place without deinitializing either one.
  ///
  ///
  /// # Examples
  ///
  /// Basic usage:
  ///
  /// ``` cpp
  /// Option x = Some(2);
  /// auto old_x = x.replace(5);
  /// ASSERT_EQ(x, Some(5));
  /// ASSERT_EQ(old_x, Some(2));
  ///
  /// Option<int> y = None;
  /// auto old_y = y.replace(3);
  /// ASSERT_EQ(y, Some(3));
  /// ASSERT_EQ(old_y, None);
  /// ```
  [[nodiscard]] auto replace(T const& replacement)->Option {
    static_assert(copy_constructible<T>);
    if (is_some()) {
      T copy = replacement;
      std::swap(copy, value_ref_());
      return Some<T>(std::move(copy));
    } else {
      new (&storage_value_) T(replacement);
      is_none_ = false;
      return None;
    }
  }

  /// Returns a copy of the option and its contents.
  ///
  /// # Examples
  ///
  /// Basic usage:
  ///
  /// ``` cpp
  /// Option x  = Some(8);
  ///
  /// ASSERT_EQ(x, x.clone());
  /// ```
  [[nodiscard]] constexpr auto clone() const->Option {
    static_assert(copy_constructible<T>);
    if (is_some()) {
      return Some<T>(std::move(T(value_cref_())));
    } else {
      return None;
    }
  }

  /// Unwraps an option, expecting `None` and returning nothing.
  ///
  /// # Panics
  ///
  /// Panics if the value is a `Some`, with a panic message.
  ///
  ///
  /// # Examples
  ///
  /// Basic usage:
  ///
  /// ``` cpp
  /// auto divide = [](double num, double denom) -> Option<double> {
  /// if (denom == 0.0) return None;
  ///  return Some(num / denom);
  /// };
  ///
  /// ASSERT_DEATH(divide(0.0, 1.0).expect_none());
  /// ASSERT_NO_THROW(divide(1.0, 0.0).expect_none());
  /// ```
  void expect_none(std::string_view const& msg)&& {
    if (is_some()) {
      internal::option::expect_none_failed(msg);
    }
  }

  /// Unwraps an option, expecting `None` and returning nothing.
  ///
  /// # Panics
  ///
  /// Panics if the value is a `Some`.
  ///
  ///
  /// # Examples
  ///
  /// Basic usage:
  ///
  /// ``` cpp
  /// auto divide = [](double num, double denom) -> Option<double> {
  /// if (denom == 0.0) return None;
  ///  return Some(num / denom);
  /// };
  ///
  /// ASSERT_DEATH(divide(0.0, 1.0).unwrap_none());
  /// ASSERT_NO_THROW(divide(1.0, 0.0).unwrap_none());
  /// ```
  void unwrap_none()&& {
    if (is_some()) {
      internal::option::no_none();
    }
  }

  /// Returns the contained value or a default of T
  ///
  /// Consumes this object and returns its `Some<T>` value if it is a `Some<T>`
  /// variant, else if this object is a `None` variant, returns the default of
  /// the value type `T`.
  ///
  ///
  /// # Examples
  ///
  /// Basic usage:
  ///
  /// ``` cpp
  /// Option x = Some("Ten"s);
  /// Option<string> y = None;
  ///
  /// ASSERT_EQ(move(x).unwrap_or_default(), "Ten"s);
  /// ASSERT_EQ(move(y).unwrap_or_default(), ""s);
  /// ```
  [[nodiscard]] constexpr auto unwrap_or_default()&&->T {
    static_assert(default_constructible<T>);
    if (is_some()) {
      return std::move(value_ref_());
    } else {
      return T();
    }
  }

  /// Calls the parameter `some_fn` with the value if this `Option` is a
  /// `Some<T>` variant, else calls `none_fn`. This `Option` is consumed
  /// afterward.
  ///
  /// The return type of both parameters must be convertible. They can also both
  /// return nothing ( `void` ).
  ///
  ///
  /// # Examples
  ///
  /// Basic usage:
  ///
  /// ``` cpp
  /// auto j = make_some("James"s).match([](string name) { return name; },
  ///                                    []() { return "<unidentified>"s; });
  /// ASSERT_EQ(j, "James"s);
  ///
  /// auto k = make_none<string>().match([](string name) { return name; },
  ///                                    []() { return "<unidentified>"s; });
  /// ASSERT_EQ(k, "<unidentified>"s);
  /// ```
  ///
  /// # Notes
  ///
  /// - The `Option`'s reference type is passed to the function arguments. i.e.
  /// If the `Option` is an r-value, r-value references are passed to the
  /// function arguments `some_fn` and `none_fn`.
  ///
  template <typename SomeFn, typename NoneFn>
  [[nodiscard]] constexpr auto match(
      SomeFn && some_fn, NoneFn && none_fn)&&->invoke_result<SomeFn&&, T&&> {
    static_assert(invocable<SomeFn&&, T&&>);
    static_assert(invocable<NoneFn&&>);

    if (is_some()) {
      return std::forward<SomeFn&&>(some_fn)(std::move(value_ref_()));
    } else {
      return std::forward<NoneFn&&>(none_fn)();
    }
  }

  template <typename SomeFn, typename NoneFn>
  [[nodiscard]] constexpr auto match(
      SomeFn && some_fn, NoneFn && none_fn)&->invoke_result<SomeFn&&, T&> {
    static_assert(invocable<SomeFn&&, T&>);
    static_assert(invocable<NoneFn&&>);

    if (is_some()) {
      return std::forward<SomeFn&&>(some_fn)(value_ref_());
    } else {
      return std::forward<NoneFn&&>(none_fn)();
    }
  }

  template <typename SomeFn, typename NoneFn>
  [[nodiscard]] constexpr auto match(SomeFn && some_fn, NoneFn && none_fn)
      const&->invoke_result<SomeFn&&, T const&> {
    static_assert(invocable<SomeFn&&, T const&>);
    static_assert(invocable<NoneFn&&>);

    if (is_some()) {
      return std::forward<SomeFn&&>(some_fn)(value_cref_());
    } else {
      return std::forward<NoneFn&&>(none_fn)();
    }
  }

 private:
  union {
    T storage_value_;
  };

  bool is_none_;

  [[nodiscard]] constexpr T& value_ref_() { return storage_value_; }

  [[nodiscard]] constexpr T const& value_cref_() const {
    return storage_value_;
  }

  template <typename Tp>
  friend Tp&& internal::option::unsafe_value_move(Option<Tp>&);
};

template <typename U, typename T>
[[nodiscard]] STX_FORCE_INLINE constexpr bool operator==(
    Some<U> const& cmp, Option<T> const& option) {
  return option == cmp;
}

template <typename U, typename T>
[[nodiscard]] STX_FORCE_INLINE constexpr bool operator!=(
    Some<U> const& cmp, Option<T> const& option) {
  return option != cmp;
}

template <typename T>
[[nodiscard]] STX_FORCE_INLINE constexpr bool operator==(
    NoneType const&, Option<T> const& option) noexcept {
  return option.is_none();
}

template <typename T>
[[nodiscard]] STX_FORCE_INLINE constexpr bool operator!=(
    NoneType const&, Option<T> const& option) noexcept {
  return option.is_some();
}

// JUST LOOK AWAY

namespace internal {
namespace result {

// constructs an r-value reference to the result's value directly, without
// checking if it is in the `Ok` or `Err` state. This is totally unsafe and
// user-end code should **never** use this
template <typename Tp, typename Er>
inline Tp&& unsafe_value_move(Result<Tp, Er>&);

// constructs an r-value reference to the result's error directly, without
// checking if it is in the `Err` or `Ok` state. This is totally unsafe and
// user-end code should **never** use this
template <typename Tp, typename Er>
inline Er&& unsafe_err_move(Result<Tp, Er>&);

}  // namespace result
}  // namespace internal

//! ### Error handling with the `Result` type.
//!
//! `Result<T, E>` is a type used for returning and propagating
//! errors. It is a class with the variants: `Ok<T>`, representing
//! success and containing a value, and `Err<E>`, representing error
//! and containing an error value.
//!
//!
//! Functions return `Result` whenever errors are expected and
//! recoverable.
//!
//! A simple function returning `Result` might be
//! defined and used like so:
//!
//! ``` cpp
//! enum class Version { Version1 = 1, Version2 = 2 };
//!
//! auto parse_version =
//!      [](array<uint8_t, 5> const& header) -> Result<Version, string_view> {
//!    switch (header.at(0)) {
//!      case 1:
//!        return Ok(Version::Version1);
//!      case 2:
//!        return Ok(Version::Version2);
//!      default:
//!        return Err("invalid version"sv);
//!    }
//!  };
//!
//! parse_version({1, 2, 3, 4, 5})
//!      .match(
//!          [](auto version) {
//!            std::cout << "Working with version: "
//!                      << static_cast<int>(version) << "\n";
//!          },
//!          [](auto err) {
//!            std::cout << "Error parsing header: " << err << "\n";
//!          });
//! ```
//!
//!
//! `Result` comes with some convenience methods that make working with it more
//! succinct.
//!
//! ``` cpp
//! Result<int, int> good_result = Ok(10);
//! Result<int, int> bad_result = Err(10);
//!
//! // The `is_ok` and `is_err` methods do what they say.
//! ASSERT_TRUE(good_result.is_ok() && !good_result.is_err());
//! ASSERT_TRUE(bad_result.is_err() && !bad_result.is_ok());
//! ```
//!
//! `Result` is a type that represents either success (`Ok`) or failure (`Err`).
//!
//! Result is either in the Ok or Err state at any point in time
//!
//! # Constexpr ?
//!
//! C++ 20 and above
//!
//! # Note
//!
//! `Result` unlike `Option` is a value-forwarding type. It doesn't have copy
//! constructors of any sort. More like a `unique_ptr`.
//!
//! `Result` should be seen as a return channel (for returning from functions)
//! and not an object.
//!
template <typename T, typename E>
struct [[nodiscard]] Result {
 public:
  static_assert(movable<T>,
                "Value type 'T' for 'Result<T, E>' must be movable");
  static_assert(movable<E>,
                "Error type 'E' for 'Result<T, E>' must be movable");
  static_assert(
      !is_reference<T>,
      "Cannot use a reference for value type 'T' of 'Result<T, E>', To prevent "
      "subtleties use "
      "type wrappers like std::reference_wrapper (stx::Ref) or any of the "
      "`stx::ConstRef` or `stx::MutRef` specialized aliases instead");
  static_assert(
      !is_reference<E>,
      "Cannot use a reference for error type 'E' of 'Result<T, E>', To prevent "
      "subtleties use "
      "type wrappers like std::reference_wrapper (stx::Ref) or any of the "
      "`stx::ConstRef` or `stx::MutRef` specialized aliases instead");

  using value_type = T;
  using error_type = E;

  constexpr Result(Ok<T> && result)
      : storage_value_(std::forward<T>(result.value_)), is_ok_(true) {}

  constexpr Result(Err<E> && err)
      : storage_err_(std::forward<E>(err.value_)), is_ok_(false) {}

  // not possible as constexpr yet:
  // 1 - we need to check which variant is present
  // 2 - the union will be default-constructed (empty) and we thus need to call
  // placement-new in the constructor block
  Result(Result && rhs) : is_ok_(rhs.is_ok_) {
    if (rhs.is_ok()) {
      new (&storage_value_) T(std::move(rhs.storage_value_));
    } else {
      new (&storage_err_) E(std::move(rhs.storage_err_));
    }
  }

  Result& operator=(Result&& rhs) {
    if (is_ok() && rhs.is_ok()) {
      std::swap(value_ref_(), rhs.value_ref_());
    } else if (is_ok() && rhs.is_err()) {
      // we need to place a new value in here (discarding old value)
      storage_value_.~T();
      new (&storage_err_) E(std::move(rhs.storage_err_));
      is_ok_ = false;
    } else if (is_err() && rhs.is_ok()) {
      storage_err_.~E();
      new (&storage_value_) T(std::move(rhs.storage_value_));
      is_ok_ = true;
    } else {
      // both are errs
      std::swap(err_ref_(), rhs.err_ref_());  // NOLINT
    }
    return *this;
  }

  Result() = delete;
  Result(Result const& rhs) = delete;
  Result& operator=(Result const& rhs) = delete;

  STX_CXX20_DESTRUCTOR_CONSTEXPR ~Result() noexcept {
    if (is_ok()) {
      storage_value_.~T();
    } else {
      storage_err_.~E();
    }
  };

  template <typename U>
  [[nodiscard]] constexpr bool operator==(Ok<U> const& cmp) const {
    static_assert(equality_comparable<T, U>);
    if (is_ok()) {
      return value_cref_() == cmp.value();
    } else {
      return false;
    }
  }

  template <typename U>
  [[nodiscard]] constexpr bool operator!=(Ok<U> const& cmp) const {
    static_assert(equality_comparable<T, U>);
    if (is_ok()) {
      return value_cref_() != cmp.value();
    } else {
      return true;
    }
  }

  template <typename F>
  [[nodiscard]] constexpr bool operator==(Err<F> const& cmp) const {
    static_assert(equality_comparable<E, F>);
    if (is_ok()) {
      return false;
    } else {
      return err_cref_() == cmp.value();
    }
  }

  template <typename F>
  [[nodiscard]] constexpr bool operator!=(Err<F> const& cmp) const {
    static_assert(equality_comparable<E, F>);
    if (is_ok()) {
      return true;
    } else {
      return err_cref_() != cmp.value();
    }
  }

  template <typename U, typename F>
  [[nodiscard]] constexpr bool operator==(Result<U, F> const& cmp) const {
    static_assert(equality_comparable<T, U>);
    static_assert(equality_comparable<E, F>);

    if (is_ok() && cmp.is_ok()) {
      return value_cref_() == cmp.value_cref_();
    } else if (is_err() && cmp.is_err()) {
      return err_cref_() == cmp.err_cref_();
    } else {
      return false;
    }
  }

  template <typename U, typename F>
  [[nodiscard]] constexpr bool operator!=(Result<U, F> const& cmp) const {
    static_assert(equality_comparable<T, U>);
    static_assert(equality_comparable<E, F>);

    if (is_ok() && cmp.is_ok()) {
      return value_cref_() != cmp.value_cref_();
    } else if (is_err() && cmp.is_err()) {
      return err_cref_() != cmp.err_cref_();
    } else {
      return true;
    }
  }

  /// Returns `true` if the result is an `Ok<T>` variant.
  ///
  ///
  /// # Examples
  ///
  /// Basic usage:
  ///
  /// ``` cpp
  /// Result<int, string_view> x = Ok(-3);
  /// ASSERT_TRUE(x.is_ok());
  ///
  /// Result<int, string_view> y = Err("Some error message"sv);
  /// ASSERT_FALSE(y.is_ok());
  /// ```
  [[nodiscard]] constexpr bool is_ok() const noexcept { return is_ok_; }

  /// Returns `true` if the result is `Err<T>`.
  ///
  /// # Examples
  ///
  /// Basic usage:
  ///
  /// ``` cpp
  /// Result<int, string_view> x = Ok(-3);
  /// ASSERT_FALSE(x.is_err());
  ///
  /// Result<int, string_view> y = Err("Some error message"sv);
  /// ASSERT_TRUE(y.is_err());
  /// ```
  [[nodiscard]] constexpr bool is_err() const noexcept { return !is_ok(); }

  [[nodiscard]] operator bool() const noexcept { return is_ok(); }

  /// Returns `true` if the result is an `Ok<T>` variant and contains the given
  /// value.
  ///
  /// # Examples
  ///
  /// Basic usage:
  ///
  /// ``` cpp
  ///
  /// Result<int, string> x = Ok(2);
  /// ASSERT_TRUE(x.contains(2));
  ///
  /// Result<int, string> y = Ok(3);
  /// ASSERT_FALSE(y.contains(2));
  ///
  /// Result<int, string> z = Err("Some error message"s);
  /// ASSERT_FALSE(z.contains(2));
  /// ```
  template <typename CmpType>
  [[nodiscard]] constexpr bool contains(CmpType const& cmp) const {
    static_assert(equality_comparable<T, CmpType>);
    if (is_ok()) {
      return value_cref_() == cmp;
    } else {
      return false;
    }
  }

  /// Returns `true` if the result is an `Err<E>` variant containing the given
  /// value.
  ///
  /// # Examples
  ///
  /// Basic usage:
  ///
  /// ``` cpp
  ///
  /// Result<int, string> x = Ok(2);
  /// ASSERT_FALSE(x.contains_err("Some error message"s));
  ///
  /// Result<int, string> y = Err("Some error message"s);
  /// ASSERT_TRUE(y.contains_err("Some error message"s));
  ///
  /// Result<int, string> z = Err("Some other error message"s);
  /// ASSERT_FALSE(z.contains_err("Some error message"s));
  /// ```
  template <typename ErrCmp>
  [[nodiscard]] constexpr bool contains_err(ErrCmp const& cmp) const {
    static_assert(equality_comparable<E, ErrCmp>);
    if (is_ok()) {
      return false;
    } else {
      return err_cref_() == cmp;
    }
  }

  /// Returns the value of evaluating the `predicate` on the contained value if
  /// the `Option` is a `Some`, else returns `false`.
  ///
  /// # Examples
  ///
  /// Basic usage:
  ///
  /// ``` cpp
  /// Result<int, string> x = Ok(2);
  /// auto even = [](auto x) { return x == 2; };
  ///
  /// ASSERT_TRUE(x.exists(even));
  ///
  /// ```
  template <typename UnaryPredicate>
  [[nodiscard]] constexpr bool exists(UnaryPredicate && predicate) const {
    static_assert(invocable<UnaryPredicate&&, T const&>);
    static_assert(convertible<invoke_result<UnaryPredicate&&, T const&>, bool>);

    if (is_ok()) {
      return std::forward<UnaryPredicate&&>(predicate)(value_cref_());
    } else {
      return false;
    }
  }

  /// Returns the value of evaluating the `predicate` on the contained value if
  /// the `Option` is a `Some`, else returns `false`.
  ///
  /// # Examples
  ///
  /// Basic usage:
  ///
  /// ``` cpp
  /// Result<int, string> x = Err("invalid"s);
  /// auto invalid = [](auto x) { return x == "invalid"; };
  ///
  /// ASSERT_TRUE(x.err_exists(invalid));
  ///
  /// ```
  template <typename UnaryPredicate>
  [[nodiscard]] constexpr bool err_exists(UnaryPredicate && predicate) const {
    static_assert(invocable<UnaryPredicate&&, E const&>);
    static_assert(convertible<invoke_result<UnaryPredicate&&, E const&>, bool>);

    if (is_err()) {
      return std::forward<UnaryPredicate&&>(predicate)(err_cref_());
    } else {
      return false;
    }
  }

  /// Returns an l-value reference to the contained value.
  /// Note that no copying occurs here.
  ///
  /// # Panics
  ///
  /// Panics if the value is an `Err`
  ///
  /// # Examples
  ///
  /// Basic usage:
  ///
  /// ``` cpp
  /// auto result = make_ok<int, int>(6);
  /// int& value = result.value();
  /// value = 97;
  ///
  /// ASSERT_EQ(result, Ok(97));
  /// ```
  [[nodiscard]] T& value()& noexcept {
    if (is_err()) internal::result::no_lref(err_cref_());
    return value_ref_();
  }

  /// Returns an l-value reference to the contained value.
  /// Note that no copying occurs here.
  ///
  /// # Panics
  ///
  /// Panics if the value is an `Err`
  ///
  /// # Examples
  ///
  /// Basic usage:
  ///
  /// ``` cpp
  /// auto const result = make_ok<int, int>(6);
  /// int const& value = result.value();
  ///
  /// ASSERT_EQ(value, 6);
  /// ```
  [[nodiscard]] T const& value() const& noexcept {
    if (is_err()) internal::result::no_lref(err_cref_());
    return value_cref_();
  }

  /// Use `unwrap()` instead
  [[deprecated("Use `unwrap()` instead")]] T value()&& = delete;
  /// Use `unwrap()` instead
  [[deprecated("Use `unwrap()` instead")]] T const value() const&& = delete;

  /// Returns an l-value reference to the contained error value.
  /// Note that no copying occurs here.
  ///
  /// # Panics
  ///
  /// Panics if the value is an `Ok`
  ///
  /// # Examples
  ///
  /// Basic usage:
  ///
  /// ``` cpp
  /// auto result = make_err<int, int>(9);
  /// int& err = result.err_value();
  /// err = 46;
  ///
  /// ASSERT_EQ(result, Err(46));
  /// ```
  [[nodiscard]] E& err_value()& noexcept {
    if (is_ok()) internal::result::no_err_lref();
    return err_ref_();
  }

  /// Returns a const l-value reference to the contained error value.
  /// Note that no copying occurs here.
  ///
  /// # Panics
  ///
  /// Panics if the value is an `Ok`
  ///
  /// # Examples
  ///
  /// Basic usage:
  ///
  /// ``` cpp
  /// auto const result = make_err<int, int>(9);
  /// int const& err = result.err_value();
  ///
  /// ASSERT_EQ(err, 9);
  /// ```
  [[nodiscard]] E const& err_value() const& noexcept {
    if (is_ok()) internal::result::no_err_lref();
    return err_cref_();
  }

  /// Use `unwrap_err()` instead
  [[deprecated("Use `unwrap_err()` instead")]] E err_value()&& = delete;
  /// Use `unwrap_err()` instead
  [[deprecated("Use `unwrap_err()` instead")]] E const err_value() const&& =
      delete;

  /// Converts from `Result<T, E>` to `Option<T>`.
  ///
  /// Converts this result into an `Option<T>`, consuming itself,
  /// and discarding the error, if any.
  ///
  /// # Examples
  ///
  /// Basic usage:
  ///
  /// ``` cpp
  /// Result<int, string> x = Ok(2);
  /// ASSERT_EQ(move(x).ok(), Some(2));
  ///
  /// Result<int, string> y = Err("Nothing here"s);
  /// ASSERT_EQ(move(y).ok(), None);
  /// ```
  [[nodiscard]] constexpr auto ok()&&->Option<T> {
    if (is_ok()) {
      return Some<T>(std::move(value_ref_()));
    } else {
      return None;
    }
  }

  /// Converts from `Result<T, E>` to `Option<E>`.
  ///
  /// Converts this result into an `Option<E>`, consuming itself, and discarding
  /// the success value, if any.
  ///
  /// # Examples
  ///
  /// Basic usage:
  ///
  /// ``` cpp
  /// Result<int, string> x = Ok(2);
  /// ASSERT_EQ(move(x).err(), None);
  ///
  /// Result<int, string> y = Err("Nothing here"s);
  /// ASSERT_EQ(move(y).err(), Some("Nothing here"s));
  /// ```
  [[nodiscard]] constexpr auto err()&&->Option<E> {
    if (is_ok()) {
      return None;
    } else {
      return Some<E>(std::move(err_ref_()));
    }
  }

  /// Converts from `Result<T, E> &` to `Result<ConstRef<T>, ConstRef<E>>`.
  ///
  /// Produces a new `Result`, containing an immutable reference
  /// into the original, leaving the original in place.
  ///
  /// # Examples
  ///
  /// Basic usage:
  ///
  /// ``` cpp
  /// Result<int, string> x = Ok(2);
  /// ASSERT_EQ(x.as_cref().unwrap().get(), 2);
  ///
  /// Result<int, string> y = Err("Error"s);
  /// ASSERT_EQ(y.as_cref().unwrap_err().get(), "Error"s);
  /// ```
  [[nodiscard]] constexpr auto as_cref()
      const& noexcept->Result<ConstRef<T>, ConstRef<E>> {
    if (is_ok()) {
      return Ok<ConstRef<T>>(ConstRef<T>(value_cref_()));
    } else {
      return Err<ConstRef<E>>(ConstRef<E>(err_cref_()));
    }
  }

  [[deprecated(
      "calling Result::as_cref() on an r-value, and "
      "therefore binding an l-value reference to an object that is marked to "
      "be moved")]]  //
  [[nodiscard]] constexpr auto
  as_cref() const&& noexcept->Result<ConstRef<T>, ConstRef<E>> = delete;

  /// Converts from `Result<T, E> &` to `Result<MutRef<T>, MutRef<E>>`.
  ///
  /// # Examples
  ///
  /// Basic usage:
  ///
  /// ``` cpp
  /// auto mutate = [](Result<int, int>& r) {
  ///  r.as_ref().match([](auto ok) { ok.get() = 42; },
  ///                   [](auto err) { err.get() = 0; });
  /// };
  ///
  /// Result<int, int> x = Ok(2);
  /// mutate(x);
  /// ASSERT_EQ(x, Ok(42));
  ///
  /// Result<int, int> y = Err(13);
  /// mutate(y);
  /// ASSERT_EQ(y, Err(0));
  /// ```
  [[nodiscard]] constexpr auto as_ref()& noexcept
      ->Result<MutRef<T>, MutRef<E>> {
    if (is_ok()) {
      return Ok<MutRef<T>>(MutRef<T>(value_ref_()));
    } else {
      return Err<MutRef<E>>(MutRef<E>(err_ref_()));
    }
  }

  [[nodiscard]] constexpr auto as_ref()
      const& noexcept->Result<ConstRef<T>, ConstRef<E>> {
    return as_cref();
  }

  [[deprecated(
      "calling Result::as_ref() on an r-value, and therefore binding a "
      "reference to an object that is marked to be moved")]]  //
  [[nodiscard]] constexpr auto
  as_ref()&& noexcept->Result<MutRef<T>, MutRef<E>> = delete;

  [[deprecated(
      "calling Result::as_ref() on an r-value, and therefore binding a "
      "reference to an object that is marked to be moved")]]  //
  [[nodiscard]] constexpr auto
  as_ref() const&& noexcept->Result<ConstRef<T>, ConstRef<E>> = delete;

  /// Maps a `Result<T, E>` to `Result<U, E>` by applying the function `op` to
  /// the contained `Ok<T>` value, leaving an `Err<E>` value untouched.
  ///
  /// This function can be used to compose the results of two functions.
  ///
  /// # Examples
  ///
  /// Basic usage:
  ///
  /// Extract the content-type from an http header
  ///
  /// ``` cpp
  /// enum class Error { InvalidHeader };
  /// auto header = "Content-Type: multipart/form-data"sv;
  ///
  /// auto check_header = [](string_view s) -> Result<string_view, Error> {
  ///  if (!s.starts_with("Content-Type: "sv)) return Err(Error::InvalidHeader);
  ///  return Ok(move(s));
  /// };
  ///
  /// auto content_type =
  /// check_header(header).map([](auto s) { return s.substr(14); });
  ///
  /// ASSERT_EQ(content_type, Ok("multipart/form-data"sv));
  /// ```
  template <typename Fn>
  [[nodiscard]] constexpr auto map(Fn &&
                                   op)&&->Result<invoke_result<Fn&&, T&&>, E> {
    static_assert(invocable<Fn&&, T&&>);
    if (is_ok()) {
      return Ok<invoke_result<Fn&&, T&&>>(
          std::forward<Fn&&>(op)(std::move(value_ref_())));
    } else {
      return Err<E>(std::move(err_ref_()));
    }
  }

  /// Applies a function to the contained value (if any),
  /// or returns the provided default (if not).
  ///
  /// # Examples
  ///
  /// Basic usage:
  ///
  /// ``` cpp
  /// Result<string, int> x = Ok("foo"s);
  /// auto map_fn = [](auto s) { return s.size(); };
  /// ASSERT_EQ(move(x).map_or(map_fn, 42UL), 3UL);
  ///
  /// Result<string, int> y = Err(-404);
  /// ASSERT_EQ(move(y).map_or(map_fn, 42UL), 42UL);
  /// ```
  template <typename Fn, typename AltType>
  [[nodiscard]] constexpr auto map_or(
      Fn && op, AltType && alt)&&->invoke_result<Fn&&, T&&> {
    static_assert(invocable<Fn&&, T&&>);
    if (is_ok()) {
      return std::forward<Fn&&>(op)(std::move(value_ref_()));
    } else {
      return std::forward<AltType&&>(alt);
    }
  }

  /// Maps a `Result<T, E>` to `U` by applying a function to a contained `Ok`
  /// value, or a fallback function to a contained `Err` value.
  ///
  /// This function can be used to unpack a successful result
  /// while handling an error.
  ///
  /// # Examples
  ///
  /// Basic usage:
  ///
  /// ``` cpp
  /// size_t const k = 21;
  ///
  /// Result<string_view, size_t> x = Ok("foo"sv);
  /// auto map_fn = [](auto s) { return s.size(); };
  /// auto else_fn = [&](auto) { return k * 2UL; };
  ///
  /// ASSERT_EQ(move(x).map_or_else(map_fn, else_fn), 3);
  ///
  /// Result<string_view, size_t> y = Err(404UL);
  /// ASSERT_EQ(move(y).map_or_else(map_fn, else_fn), 42);
  /// ```
  template <typename Fn, typename A>
  [[nodiscard]] constexpr auto map_or_else(
      Fn && op, A && alt_op)&&->invoke_result<Fn&&, T&&> {
    static_assert(invocable<Fn&&, T&&>);
    static_assert(invocable<A&&, E&&>);

    if (is_ok()) {
      return std::forward<Fn&&>(op)(std::move(value_ref_()));
    } else {
      return std::forward<A&&>(alt_op)(std::move(err_ref_()));
    }
  }

  /// Maps a `Result<T, E>` to `Result<T, F>` by applying a function to a
  /// contained `Err` value, leaving an `Ok` value untouched.
  ///
  /// This function can be used to pass through a successful result while
  /// handling an error.
  ///
  /// # Examples
  ///
  /// Basic usage:
  ///
  /// ``` cpp
  /// auto stringify = [](auto x) { return "error code: " + std::to_string(x);
  /// };
  ///
  /// Result<int, int> x = Ok(2);
  /// ASSERT_EQ(move(x).map_err(stringify), Ok(2));
  ///
  /// Result<int, int> y = Err(404);
  /// ASSERT_EQ(move(y).map_err(stringify), Err("error code: 404"s));
  /// ```
  template <typename Fn>
  [[nodiscard]] constexpr auto map_err(
      Fn && op)&&->Result<T, invoke_result<Fn&&, E&&>> {
    static_assert(invocable<Fn&&, E&&>);
    if (is_ok()) {
      return Ok<T>(std::move(value_ref_()));
    } else {
      return Err<invoke_result<Fn&&, E&&>>(
          std::forward<Fn&&>(op)(std::move(err_ref_())));
    }
  }

  /// Returns `res` if the result is `Ok`, otherwise returns the `Err` value
  /// of itself.
  ///
  /// # Examples
  ///
  /// Basic usage:
  ///
  /// ``` cpp
  /// Result<int, string_view> a = Ok(2);
  /// Result<string_view, string_view> b = Err("late error"sv);
  /// ASSERT_EQ(move(a).AND(move(b)), Err("late error"sv));
  ///
  /// Result<int, string_view> c = Err("early error"sv);
  /// Result<string_view, string_view> d = Ok("foo"sv);
  /// ASSERT_EQ(move(c).AND(move(d)), Err("early error"sv));
  ///
  /// Result<int, string_view> e = Err("not a 2"sv);
  /// Result<string_view, string_view> f = Err("late error"sv);
  /// ASSERT_EQ(move(e).AND(move(f)), Err("not a 2"sv));
  ///
  /// Result<int, string_view> g = Ok(2);
  /// Result<string_view, string_view> h = Ok("different result type"sv);
  /// ASSERT_EQ(move(g).AND(move(h)), Ok("different result type"sv));
  /// ```
  // a copy attempt like passing a const could cause an error
  template <typename U, typename F>
  [[nodiscard]] constexpr auto AND(Result<U, F> && res)&&->Result<U, F> {
    static_assert(convertible<E&&, F>);
    if (is_ok()) {
      return std::forward<Result<U, F>&&>(res);
    } else {
      return Err<F>(std::move(static_cast<F>(std::move(err_ref_()))));
    }
  }

  /// Calls `op` if the result is `Ok`, otherwise returns the `Err` value of
  /// itself.
  ///
  /// This function can be used for control flow based on `Result` values.
  ///
  /// # Examples
  ///
  /// Basic usage:
  ///
  /// ``` cpp
  /// auto sq = [](int x) { return x * x; };
  ///
  /// auto make_ok = [](int x) -> Result<int, int> { return Ok(move(x)); };
  /// auto make_err = [](int x) -> Result<int, int> { return Err(move(x)); };
  ///
  /// ASSERT_EQ(make_ok(2).and_then(sq).and_then(sq), Ok(16));
  /// ASSERT_EQ(make_err(3).and_then(sq).and_then(sq), Err(3));
  /// ```
  template <typename Fn>
  [[nodiscard]] constexpr auto and_then(
      Fn && op)&&->Result<invoke_result<Fn&&, T&&>, E> {
    static_assert(invocable<Fn&&, T&&>);
    if (is_ok()) {
      return Ok<invoke_result<Fn&&, T&&>>(
          std::forward<Fn&&>(op)(std::move(value_ref_())));
    } else {
      return Err<E>(std::move(err_ref_()));
    }
  }

  /// Returns `res` if the result is `Err`, otherwise returns the `Ok` value of
  /// itself.
  ///
  /// Arguments passed to `or` are eagerly evaluated; if you are passing the
  /// result of a function call, it is recommended to use `or_else`, which is
  /// lazily evaluated.
  ///
  /// # Examples
  ///
  /// Basic usage:
  ///
  /// ``` cpp
  /// Result<int, string_view> a = Ok(2);
  /// Result<int, string_view> b = Err("late error"sv);
  /// ASSERT_EQ(move(a).OR(move(b)), Ok(2));
  ///
  /// Result<int, string_view> c = Err("early error"sv);
  /// Result<int, string_view> d = Ok(2);
  /// ASSERT_EQ(move(c).OR(move(d)), Ok(2));
  ///
  /// Result<int, string_view> e = Err("not a 2"sv);
  /// Result<int, string_view> f = Err("late error"sv);
  /// ASSERT_EQ(move(e).OR(move(f)), Err("late error"sv));
  ///
  /// Result<int, string_view> g = Ok(2);
  /// Result<int, string_view> h = Ok(100);
  /// ASSERT_EQ(move(g).OR(move(h)), Ok(2));
  /// ```
  // passing a const ref will cause an error
  template <typename U, typename F>
  [[nodiscard]] constexpr auto OR(Result<U, F> && alt)&&->Result<U, F> {
    static_assert(convertible<T&&, U>);
    if (is_ok()) {
      return Ok<U>(std::move(static_cast<U>(std::move(value_ref_()))));
    } else {
      return std::forward<Result<U, F>&&>(alt);
    }
  }

  /// Calls `op` if the result is `Err`, otherwise returns the `Ok` value of
  /// itself.
  ///
  /// This function can be used for control flow based on result values.
  ///
  /// # Examples
  ///
  /// Basic usage:
  ///
  /// ``` cpp
  /// auto make_ok = [](int x) -> Result<int, int> { return Ok(move(x)); };
  /// auto make_err = [](int x) -> Result<int, int> { return Err(move(x)); };
  /// auto sq = [](int err) -> Result<int, int> { return Ok(err * err); };
  /// auto err = [](int err) -> Result<int, int> { return Err(move(err)); };
  ///
  /// ASSERT_EQ(make_ok(2).or_else(sq).or_else(sq), Ok(2));
  /// ASSERT_EQ(make_ok(2).or_else(err).or_else(sq), Ok(2));
  /// ASSERT_EQ(make_err(3).or_else(sq).or_else(err), Ok(9));
  /// ASSERT_EQ(make_err(3).or_else(err).or_else(err), Err(3));
  /// ```
  template <typename Fn>
  [[nodiscard]] constexpr auto or_else(Fn && op)&&->invoke_result<Fn&&, E&&> {
    static_assert(invocable<Fn&&, E&&>);
    if (is_ok()) {
      return Ok<T>(std::move(value_ref_()));
    } else {
      return std::forward<Fn&&>(op)(std::move(err_ref_()));
    }
  }

  /// Unwraps a result, yielding the content of an `Ok<T>` variant.
  /// Else, it returns the parameter `alt`.
  ///
  /// Arguments passed to `unwrap_or` are eagerly evaluated; if you are passing
  /// the result of a function call, it is recommended to use
  /// `unwrap_or_else`, which is lazily evaluated.
  ///
  /// # Examples
  ///
  /// Basic usage:
  ///
  /// ``` cpp
  /// int alt = 2;
  /// Result<int, string_view> x = Ok(9);
  /// ASSERT_EQ(move(x).unwrap_or(move(alt)), 9);
  ///
  /// int alt_b = 2;
  /// Result<int, string_view> y = Err("error"sv);
  /// ASSERT_EQ(move(y).unwrap_or(move(alt_b)), 2);
  /// ```
  [[nodiscard]] constexpr auto unwrap_or(T && alt)&&->T {
    if (is_ok()) {
      return std::move(value_ref_());
    } else {
      return std::move(alt);
    }
  }

  /// Unwraps a result, yielding the content of an `Ok`.
  /// If the value is an `Err` then it calls `op` with its value.
  ///
  ///
  /// # Examples
  ///
  /// Basic usage:
  ///
  /// ``` cpp
  /// auto count = [] (string_view err)  { return err.size(); };
  ///
  /// ASSERT_EQ(make_ok<size_t,string_view>(2UL).unwrap_or_else(count), 2);
  /// ASSERT_EQ(make_err<size_t,string_view>("booo"sv).unwrap_or_else(count),
  /// 4);
  /// ```
  template <typename Fn>
  [[nodiscard]] constexpr auto unwrap_or_else(Fn && op)&&->T {
    static_assert(invocable<Fn&&, E&&>);
    if (is_ok()) {
      return std::move(value_ref_());
    } else {
      return std::forward<Fn&&>(op)(std::move(err_ref_()));
    }
  }

  /// Unwraps a result, yielding the content of an `Ok`.
  ///
  /// # Panics
  ///
  /// Panics if the value is an `Err`, with a panic message provided by the
  /// `Err`'s value.
  ///
  /// # Examples
  ///
  /// Basic usage:
  ///
  /// ``` cpp
  /// ASSERT_EQ(make_ok<int, string_view>(2).unwrap(), 2);
  /// Result<int, string_view> x = Err("emergency failure"sv);
  /// ASSERT_DEATH(move(x).unwrap());
  /// ```
  [[nodiscard]] auto unwrap()&&->T {
    if (is_err()) {
      internal::result::no_value(err_cref_());
    }
    return std::move(value_ref_());
  }

  /// Unwraps a result, yielding the content of an `Ok`.
  ///
  /// # Panics
  ///
  /// Panics if the value is an `Err`, with a panic message including the
  /// passed message, and the content of the `Err`.
  ///
  /// # Examples
  ///
  /// Basic usage:
  ///
  /// ``` cpp
  /// Result<int, string_view> x = Err("emergency failure"sv);
  /// ASSERT_DEATH(move(x).expect("Testing expect"));
  /// ```
  [[nodiscard]] auto expect(std::string_view const& msg)&&->T {
    if (is_err()) {
      internal::result::expect_value_failed(msg, err_cref_());
    }
    return std::move(value_ref_());
  }

  /// Unwraps a result, yielding the content of an `Err`.
  ///
  /// # Panics
  ///
  /// Panics if the value is an `Ok`, with a custom panic message provided
  /// by the `Ok`'s value.
  ///
  ///
  /// # Examples
  ///
  /// Basic usage:
  ///
  /// ``` cpp
  /// Result<int, string_view> x = Ok(2);
  /// ASSERT_DEATH(move(x).unwrap_err()); // panics
  ///
  /// Result<int, string_view> y = Err("emergency failure"sv);
  /// ASSERT_EQ(move(y).unwrap_err(), "emergency failure");
  /// ```
  [[nodiscard]] auto unwrap_err()&&->E {
    if (is_ok()) {
      internal::result::no_err();
    }
    return std::move(err_ref_());
  }

  /// Unwraps a result, yielding the content of an `Err`.
  ///
  /// # Panics
  ///
  /// Panics if the value is an `Ok`, with a panic message including the
  /// passed message, and the content of the `Ok`.
  ///
  /// # Examples
  ///
  /// Basic usage:
  ///
  /// ``` cpp
  /// Result<int, string_view> x = Ok(10);
  /// ASSERT_DEATH(move(x).expect_err("Testing expect_err")); // panics with
  ///                                                         // "Testing
  ///                                                         // expect_err:
  ///                                                         // 10"
  /// ```
  [[nodiscard]] auto expect_err(std::string_view const& msg)&&->E {
    if (is_ok()) {
      internal::result::expect_err_failed(msg);
    }
    return std::move(err_ref_());
  }

  /// Returns the contained value or a default
  ///
  /// Consumes itself then, if `Ok`, returns the contained
  /// value, otherwise if `Err`, returns the default value for that
  /// type.
  ///
  /// # Examples
  ///
  /// Basic usage:
  ///
  /// ``` cpp
  /// Result<string, int> good_year = Ok("1909"s);
  /// Result<string, int> bad_year = Err(-1);
  ///
  /// ASSERT_EQ(move(good_year).unwrap_or_default(), "1909"s);
  /// ASSERT_EQ(move(bad_year).unwrap_or_default(), ""s); // empty string (""s)
  ///                                                     // is the default
  ///                                                     // value
  ///                                                     // for a C++ string
  /// ```
  [[nodiscard]] constexpr auto unwrap_or_default()&&->T {
    static_assert(default_constructible<T>);
    if (is_ok()) {
      return std::move(value_ref_());
    } else {
      return T();
    }
  }

  /// Calls the parameter `ok_fn` with the value if this result is an `Ok<T>`,
  /// else calls `err_fn` with the error. This result is consumed afterward.
  ///
  /// The return type of both parameters must be convertible. They can also both
  /// return nothing ( `void` ).
  ///
  ///
  /// # Examples
  ///
  /// Basic usage:
  ///
  /// ``` cpp
  /// auto i = make_ok<int, string_view>(99);
  ///
  /// auto j = move(i).match([](int value) { return value; },
  ///                        [](string_view) { return -1; });
  /// ASSERT_EQ(j, 99);
  ///
  ///
  /// auto x = make_err<int, string_view>("404 Not Found"sv);
  /// // you can return nothing (void)
  /// x.match([](int&) {},
  ///         [](string_view& s) { std::cout << "Error: " << s << "\n"; });
  /// ```
  ///
  /// # Notes
  ///
  /// - The `Result`'s reference type is passed to the function arguments. i.e.
  /// If the `Result` is an r-value, r-value references are passed to the
  /// function arguments `some_fn` and `none_fn`.
  ///
  template <typename OkFn, typename ErrFn>
  [[nodiscard]] constexpr auto match(
      OkFn && ok_fn, ErrFn && err_fn)&&->invoke_result<OkFn&&, T&&> {
    static_assert(invocable<OkFn&&, T&&>);
    static_assert(invocable<ErrFn&&, E&&>);

    if (is_ok()) {
      return std::forward<OkFn&&>(ok_fn)(std::move(value_ref_()));
    } else {
      return std::forward<ErrFn&&>(err_fn)(std::move(err_ref_()));
    }
  }

  template <typename OkFn, typename ErrFn>
  [[nodiscard]] constexpr auto match(
      OkFn && ok_fn, ErrFn && err_fn)&->invoke_result<OkFn&&, T&> {
    static_assert(invocable<OkFn&&, T&>);
    static_assert(invocable<ErrFn&&, E&>);

    if (is_ok()) {
      return std::forward<OkFn&&>(ok_fn)(value_ref_());
    } else {
      return std::forward<ErrFn&&>(err_fn)(err_ref_());
    }
  }

  template <typename OkFn, typename ErrFn>
  [[nodiscard]] constexpr auto match(OkFn && ok_fn, ErrFn && err_fn)
      const&->invoke_result<OkFn&&, T const&> {
    static_assert(invocable<OkFn&&, T const&>);
    static_assert(invocable<ErrFn&&, E const&>);

    if (is_ok()) {
      return std::forward<OkFn&&>(ok_fn)(value_cref_());
    } else {
      return std::forward<ErrFn&&>(err_fn)(err_cref_());
    }
  }

  /// Returns a copy of the result and its contents.
  ///
  /// # Examples
  ///
  /// Basic usage:
  ///
  /// ``` cpp
  /// Result<int, int> x  = Ok(8);
  ///
  /// ASSERT_EQ(x, x.clone());
  /// ```
  [[nodiscard]] constexpr auto clone() const->Result<T, E> {
    static_assert(copy_constructible<T>);
    static_assert(copy_constructible<E>);

    if (is_ok()) {
      return Ok<T>(std::move(T(value_cref_())));
    } else {
      return Err<E>(std::move(E(err_cref_())));
    }
  }

 private:
  union {
    T storage_value_;
    E storage_err_;
  };

  bool is_ok_;

  [[nodiscard]] constexpr T& value_ref_() noexcept { return storage_value_; }

  [[nodiscard]] constexpr T const& value_cref_() const noexcept {
    return storage_value_;
  }

  [[nodiscard]] constexpr E& err_ref_() noexcept { return storage_err_; }

  [[nodiscard]] constexpr E const& err_cref_() const noexcept {
    return storage_err_;
  }

  template <typename Tp, typename Er>
  friend Tp&& internal::result::unsafe_value_move(Result<Tp, Er>&);

  template <typename Tp, typename Er>
  friend Er&& internal::result::unsafe_err_move(Result<Tp, Er>&);
};

template <typename U, typename T, typename E>
[[nodiscard]] STX_FORCE_INLINE constexpr bool operator==(
    Ok<U> const& cmp, Result<T, E> const& result) {
  return result == cmp;
}

template <typename U, typename T, typename E>
[[nodiscard]] STX_FORCE_INLINE constexpr bool operator!=(
    Ok<U> const& cmp, Result<T, E> const& result) {
  return result != cmp;
}

template <typename F, typename T, typename E>
[[nodiscard]] STX_FORCE_INLINE constexpr bool operator==(
    Err<F> const& cmp, Result<T, E> const& result) {
  return result == cmp;
}

template <typename F, typename T, typename E>
[[nodiscard]] STX_FORCE_INLINE constexpr bool operator!=(
    Err<F> const& cmp, Result<T, E> const& result) {
  return result != cmp;
}

/*********************    HELPER FUNCTIONS    *********************/

/// Helper function to construct an `Option<T>` with a `Some<T>` value.
/// if the template parameter is not specified, it is auto-deduced from the
/// parameter's value.
///
/// # Examples
///
/// Basic usage:
///
///
/// ``` cpp
/// // these are some of the various ways to construct on Option<T> with a
/// // Some<T> value
/// Option g = Some(9);
/// Option h = Some<int>(9);
/// Option<int> i = Some(9);
/// auto j = Option(Some(9));
/// auto k = Option<int>(Some<int>(9));
/// auto l = Option<int>(Some(9));
/// // ... and a few more
///
/// // to make it easier and less verbose:
/// auto m = make_some(9); // 'm' is of type Option<int>
/// ASSERT_EQ(m, Some(9));
///
/// auto n = make_some<int>(9); // to be explicit
/// ASSERT_EQ(m, Some(9));
///
/// ```
///
/// # Constexpr ?
///
/// C++ 20 and above
///
template <typename T>
[[nodiscard]] STX_FORCE_INLINE constexpr auto make_some(T value) -> Option<T> {
  return Some<T>(std::forward<T>(value));
}

/// Helper function to construct an `Option<T>` with a `None` value.
/// note that the value parameter `T` must be specified.
///
/// # Examples
///
/// Basic usage:
///
///
/// ``` cpp
/// // these are some of the various ways to construct on Option<T> with
/// // a None value
/// Option<int> h = None;
/// auto i = Option<int>(None);
/// Option j = Option<int>(None);
/// Option<int> k = make_none<int>();
///
/// // to make it easier and less verbose:
/// auto m = make_none<int>();  // 'm' = Option<int>
/// ASSERT_EQ(m, None);
///
/// ```
///
/// # Constexpr ?
///
/// C++ 20 and above
///
template <typename T>
[[nodiscard]] STX_FORCE_INLINE constexpr auto make_none() noexcept
    -> Option<T> {
  return None;
}

/// Helper function to construct a `Result<T, E>` with an `Ok<T>` value.
///
/// # NOTE
///
/// The value type `T` must be specified and is the first template
/// parameter.
///
/// # Examples
///
/// Basic usage:
///
///
/// ``` cpp
/// // these are some of the various ways to construct on Result<T, E> with an
/// // Ok<T> value
/// Result<int, string> a = Ok(8);
/// Result<int, string> b = Ok<int>(8);
///
/// // to make it easier and less verbose:
/// auto c = make_ok<int, string>(9); // 'c' = Result<string, int>
/// ASSERT_EQ(c, Ok(9));
///
/// auto d = make_ok<string, int>("Hello"s);
/// ASSERT_EQ(d, Ok("Hello"s));
///
/// ```
///
/// # Constexpr ?
///
/// C++ 20 and above
///
template <typename T, typename E>
[[nodiscard]] STX_FORCE_INLINE constexpr auto make_ok(T value) -> Result<T, E> {
  return Ok<T>(std::forward<T>(value));
}

/// Helper function to construct a `Result<T, E>` with an `Err<E>` value.
/// if the template parameter `E` is not specified, it is auto-deduced from the
/// parameter's value.
///
/// # NOTE
/// The value type `T` must be specified and is the first template
/// parameter.
///
/// # Examples
///
/// Basic usage:
///
///
/// ``` cpp
///
/// // these are some of the various ways to construct on Result<T, E> with an
/// // Err<E> value
/// Result<int, string> a = Err("foo"s);
/// Result<int, string> b = Err<string>("foo"s);
///
/// // to make it easier and less verbose:
/// auto c = make_err<int, string>("bar"s); // 'c' = Result<int, string>
/// ASSERT_EQ(c, Err("bar"s));
///
/// ```
///
/// # Constexpr ?
///
/// C++ 20 and above
///
template <typename T, typename E>
[[nodiscard]] STX_FORCE_INLINE constexpr auto make_err(E err) -> Result<T, E> {
  return Err<E>(std::forward<E>(err));
}

/// Helper function to construct a `Some` containing a `std::reference_wrapper`
/// (stx::Ref)
///
/// # Examples
///
/// Basic usage:
///
/// ``` cpp
/// int x = 4;
/// Option<Ref<int>> y = some_ref(x); // constructs a
///                                   // Some<std::reference_wrapper<int>>
///                                   // a.k.a. Some<Ref<int>>
///
/// int const a = 5;
/// Option<Ref<const int>> b = some_ref(a); // constructs a
///                               // Some<std::reference_wrapper<const int>>
///                               // a.k.a. Some<Ref<const int>>
///                               // note that 'a' is const
/// ```
///
template <typename T>
STX_FORCE_INLINE auto some_ref(T& value) noexcept {
  return Some<Ref<T>>(std::forward<T&>(value));
}

/// Helper function to construct an `Ok` containing a `std::reference_wrapper`
/// (stx::Ref)
///
/// # Examples
///
/// Basic usage:
///
/// ``` cpp
/// int x = 4;
/// Result<Ref<int>, int> y = ok_ref(x); // constructs an
///                                      // Ok<std::reference_wrapper<int>>
///                                      // a.k.a. Ok<Ref<int>>
///
/// int const a = 5;
/// Result<Ref<const int>, int> b = ok_ref(a); // constructs an
///                        // Ok<std::reference_wrapper<const int>> a.k.a.
///                        // Ok<Ref<const int>
///                        // note that 'a' is const
/// ```
///
template <typename T>
STX_FORCE_INLINE auto ok_ref(T& value) noexcept {
  return Ok<Ref<T>>(std::forward<T&>(value));
}

/// Helper function to construct an `Err` containing a `std::reference_wrapper`
/// (stx::Ref)
///
/// # Examples
///
/// Basic usage:
///
/// ``` cpp
/// int x = 4;
/// Result<int, Ref<int>> y = err_ref(x); // constructs an
///                                       // Err<std::reference_wrapper<int>>
///                                       // a.k.a. Err<Ref<int>>
///
/// int const a = 5;
/// Result<int, Ref<const int>> b = err_ref(a); // constructs an
///                            // Err<std::reference_wrapper<const int>>
///                            // a.k.a. Err<Ref<const int>>
///                            // note that 'a' is const
/// ```
///
template <typename E>
STX_FORCE_INLINE auto err_ref(E& value) noexcept {
  return Err<Ref<E>>(std::forward<E&>(value));
}

STX_END_NAMESPACE

// Error propagation macros
#include "stx/internal/try.h"
