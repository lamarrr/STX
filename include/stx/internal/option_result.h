/**
 * @file option_result.h
 * @author Basit Ayantunde <rlamarrr@gmail.com>
 * @brief
 * @version  0.1
 * @date 2020-04-16
 *
 * @copyright Copyright (c) 2020
 *
 */
#ifndef STX_OPTION_RESULT_H_
#define STX_OPTION_RESULT_H_

#include <type_traits>
#include <utility>

#include "stx/common.h"
#include "stx/internal/panic_helpers.h"
#include "stx/internal/storage.h"

/// why so long? Option and Result both depend on each other. I don't know of a
/// way to break the cyclic dependency, majorly because they are templated

/// [tests_prelude]
/// to run tests, use:
/// ```
/// #include <iostream>
/// #include <string>
/// #include <string_view>
///
/// #include <fmt/format.h>
///
///
/// using std::move, std::string, std::string_view;
/// using namespace std::string_literals; // makes `"Hello"s` give std::string
///                                       // directly and not `const char []`
/// using namespace std::string_view_literals;
/// ```
namespace stx {

/// universal state-variant Type for `Option<T>` representing no-value
class NoneType {
 public:
  constexpr NoneType() = default;
  constexpr NoneType(NoneType const&) = default;
  constexpr NoneType(NoneType&&) = default;
  constexpr NoneType& operator=(NoneType const&) = default;
  constexpr NoneType& operator=(NoneType&&) = default;
  constexpr ~NoneType() noexcept = default;

  constexpr bool operator==(NoneType const&) const { return true; }
};

// universal value for `Option<T>` representing no-value
constexpr NoneType None = NoneType{};

/// value variant for `Option<T>` wrapping the contained value
template <Swappable T>
struct Some {
  static_assert(!std::is_reference_v<T>,
                "Cannot use T& for type, To prevent subtleties use "
                "reference_wrapper<T> instead");
  static_assert(!same_as<T, NoneType>,
                "Cannot use NoneType for T of Some<T>, as "
                "Option<T>(=Option<NoneType>) will have ambigous overloads");

  using value_type = T;

  /// a `Some<T>` can only be constructed with an r-value of type `T`
  explicit Some(T&& value) : value_{std::forward<T>(value)} {}

  Some(Some const&) = default;
  Some(Some&& rhs) = default;
  Some& operator=(Some const&) = default;
  Some& operator=(Some&& rhs) = default;
  ~Some() = default;

  /// get an immutable reference to the wrapped value
  T const& value() const { return value_; }

  /// get mutable reference to the wrapped value
  T& value() { return value_; }

  bool operator==(Some const& cmp) const requires EqualityComparable<T> {
    return value() == cmp.value();
  }

  bool operator==(Some<MutRef<T>> const& cmp) const
      requires EqualityComparable<T> {
    return value() == cmp.value().get();
  }

  bool operator==(Some<ConstRef<T>> const& cmp) const
      requires EqualityComparable<T> {
    return value() == cmp.value().get();
  }

  bool operator==(Some<T*> const& cmp) const requires EqualityComparable<T> {
    return value() == *cmp.value();
  }

  bool operator==(Some<T const*> const& cmp) const
      requires EqualityComparable<T> {
    return value() == *cmp.value();
  }

  bool operator==(NoneType const&) const { return false; }

 private:
  T value_;
  template <Swappable Tp>
  friend class Option;
};

/// value variant for `Result<T, E>` wrapping the contained value
template <Swappable T>
struct Ok {
  static_assert(!std::is_reference_v<T>,
                "Cannot use T& for type, To prevent subtleties use "
                "std::reference_wrapper<T> instead");
  using value_type = T;

  ///  an `Ok<T>` can only be constructed with an r-value of type `T`
  explicit Ok(T&& value) : value_{std::forward<T>(value)} {}

  Ok(Ok const&) = delete;
  Ok(Ok&& rhs) : value_{std::forward<T>(rhs.value_)} {}
  Ok& operator=(Ok const&) = delete;
  Ok& operator=(Ok&& rhs) {
    std::swap(value_, rhs.value_);
    return *this;
  }
  ~Ok() = default;

  bool operator==(Ok const& cmp) const requires EqualityComparable<T> {
    return value() == cmp.value();
  }

  bool operator==(Ok<ConstRef<T>> const& cmp) const
      requires EqualityComparable<T> {
    return value() == cmp.value().get();
  }

  bool operator==(Ok<MutRef<T>> const& cmp) const
      requires EqualityComparable<T> {
    return value() == cmp.value().get();
  }

  bool operator==(Ok<T*> const& cmp) const requires EqualityComparable<T> {
    return value() == *cmp.value();
  }

  bool operator==(Ok<const T*> const& cmp) const
      requires EqualityComparable<T> {
    return value() == *cmp.value();
  }

  /// get an immutable reference to the wrapped value
  T const& value() const { return value_; }

  /// get a mutable reference to the wrapped value
  T& value() { return value_; }

 private:
  T value_;
  template <Swappable Tp, Swappable Err>
  friend class Result;
};

/// error-value variant for `Result<T, E>` wrapping the contained error
template <Swappable E>
struct Err {
  static_assert(!std::is_reference_v<E>,
                "Cannot use T& for type, To prevent subtleties use "
                "reference_wrapper<E> instead");
  using value_type = E;

  // an `Err<E>` can only be constructed with an r-value of type `E`
  explicit Err(E&& value) : value_{std::forward<E>(value)} {}

  Err(Err const&) = delete;
  Err(Err&& rhs) : value_{std::forward<E>(rhs.value_)} {}
  Err& operator=(Err const&) = delete;
  Err& operator=(Err&& rhs) {
    std::swap(value_, rhs.value_);
    return *this;
  }
  ~Err() = default;

  bool operator==(Err const& cmp) const requires EqualityComparable<E> {
    return value() == cmp.value();
  }

  bool operator==(Err<ConstRef<E>> const& cmp) const
      requires EqualityComparable<E> {
    return value() == cmp.value().get();
  }

  bool operator==(Err<MutRef<E>> const& cmp) const
      requires EqualityComparable<E> {
    return value() == cmp.value().get();
  }

  bool operator==(Err<E*> const& cmp) const requires EqualityComparable<E> {
    return value() == *cmp.value();
  }

  bool operator==(Err<E const*> const& cmp) const
      requires EqualityComparable<E> {
    return value() == *cmp.value();
  }

  /// get an immutable reference to the wrapped value
  E const& value() const { return value_; }

  /// get a mutable reference to the wrapped value
  E& value() { return value_; }

 private:
  E value_;
  template <Swappable Tp, Swappable Err>
  friend class Result;
};

template <Swappable T, Swappable E>
class Result;

//! [section="Option"]
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
//! ```Option`s are commonly paired with pattern matching to query the
//! presence of a value and take action, always accounting for the ```None```
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
//! move(result).match([](double value) { fmt::print("{}\n", value); },
//!                     []() { fmt::print("has no value"); });
//! ```
//!
//!
template <Swappable T>
class Option {
  using value_type = T;
  using storage_type = option_variant_storage<Some<T>>;

  static_assert(!same_as<T, NoneType>,
                "Cannot use NoneType for T of Option<T>, as "
                "Option<NoneType> will have ambigous overloads");

 public:
  Option() = delete;

  Option(Some<T>&& some) {  // NOLINT
    construct_some_(std::forward<Some<T>>(some));
    is_none_ = false;
  }

  Option(NoneType const&) {  // NOLINT
    is_none_ = true;
  }

  Option(Option const&) = delete;
  Option& operator=(Option const&) = delete;

  Option(Option&& rhs) {
    if (rhs.is_some()) {
      construct_some_(std::move(rhs.value_wrap_ref_()));
    }
    is_none_ = std::move(rhs.is_none_);
  }

  Option& operator=(Option&& rhs) {
    // contained object is destroyed as appropriate after the end
    // of this scope
    std::swap(value_wrap_ref_(), rhs.value_wrap_ref_());
    std::swap(is_none_, rhs.is_none_);

    return *this;
  }

  bool operator==(Option const& cmp) const requires EqualityComparable<T> {
    if (is_some() && cmp.is_some()) {
      return value_cref_() == cmp.value_cref_();
    } else if (is_none() && cmp.is_none()) {
      return true;
    } else {
      return false;
    }
  }

  bool operator==(Some<T> const& cmp) const requires EqualityComparable<T> {
    if (is_some()) {
      return value_cref_() == cmp.value();
    } else {
      return false;
    }
  }

  bool operator==(Some<ConstRef<T>> const& cmp) const
      requires EqualityComparable<T> {
    if (is_some()) {
      return value_cref_() == cmp.value().get();
    } else {
      return false;
    }
  }

  bool operator==(Some<MutRef<T>> const& cmp) const
      requires EqualityComparable<T> {
    if (is_some()) {
      return value_cref_() == cmp.value().get();
    } else {
      return false;
    }
  }

  bool operator==(Some<const T*> const& cmp) const
      requires EqualityComparable<T> {
    if (is_some()) {
      return value_cref_() == *cmp.value();
    } else {
      return false;
    }
  }

  bool operator==(Some<T*> const& cmp) const requires EqualityComparable<T> {
    if (is_some()) {
      return value_cref_() == *cmp.value();
    } else {
      return false;
    }
  }

  bool operator==(NoneType const&) const { return is_none(); }

  ~Option() noexcept {
    if (is_some()) {
      value_wrap_ref_().~Some<T>();
    }
  }

  /// Returns `true` if this Option is a `Some` value.
  ///
  /// # Examples
  ///
  /// ```
  /// Option x = Some(2);
  /// ASSERT_TRUE(x.is_some());
  ///
  /// Option<int> y = None;
  /// ASSERT_FALSE(y.is_some());
  /// ```
  ///
  bool is_some() const noexcept { return !is_none(); }

  /// Returns `true` if the option is a `None` value.
  ///
  /// # Examples
  ///
  /// ```
  /// Option x = Some(2);
  /// ASSERT_FALSE(x.is_none());
  ///
  /// Option<int> y = None;
  /// ASSERT_TRUE(y.is_none());
  /// ```
  bool is_none() const noexcept { return is_none_; }

  /// Returns `true` if the option is a `Some` value containing the given
  /// value.
  ///
  /// # Examples
  ///
  /// ```
  /// Option x = Some(2);
  /// ASSERT_TRUE(x.contains(2));
  ///
  /// Option y = Some(3);
  /// ASSERT_FALSE(y.contains(2));
  ///
  /// Option<int> z = None;
  /// ASSERT_FALSE(z.contains(2));
  /// ```
  template <EqualityComparable<T const&> CmpType>
  bool contains(CmpType const& cmp) const {
    if (is_some()) {
      return value_cref_() == cmp;
    } else {
      return false;
    }
  }

  /// Converts from `Option<T> const&` or `Option<T> &` to
  /// `Option<ConstRef<T>>`.
  ///
  /// NOTE: `ConstRef<T>` is an alias for `std::reference_wrapper<T const>` and
  /// guides against reference-collapsing
  auto as_const_ref() const& -> Option<ConstRef<T>> {
    if (is_some()) {
      return Some(ConstRef<T>(value_cref_()));
    } else {
      return None;
    }
  }

  [
      [deprecated("calling Option::as_const_ref() on an r-value (temporary), "
                  "therefore binding a reference to an object that is about to "
                  "be destroyed")]]  //
  auto
  as_const_ref() const&& -> Option<ConstRef<T>> {
    if (is_some()) {
      return Some(ConstRef<T>(value_cref_()));
    } else {
      return None;
    }
  }

  /// Converts from `Option<T>` to `Option<MutRef<T>>`.
  ///
  /// # Examples
  ///
  /// ```
  /// auto mutate = [](Option<int>& r) {
  ///  r.as_mut_ref().match([](MutRef<int> ref) { ref.get() = 42; },
  ///                       []() { });
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
  auto as_mut_ref() & -> Option<MutRef<T>> {
    if (is_some()) {
      return Some(MutRef<T>(value_ref_()));
    } else {
      return None;
    }
  }

  [
      [deprecated("calling Option::as_mut_ref() on an r-value (temporary), "
                  "therefore binding a "
                  "reference to an object that is about to be destroyed")]]  //
  auto
  as_mut_ref() && -> Option<MutRef<T>> {
    if (is_some()) {
      return Some(MutRef<T>(value_ref_()));
    } else {
      return None;
    }
  }

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
  /// ```
  /// Option x = Some("value"s);
  /// ASSERT_EQ(move(x).expect("the world is ending"), "value");
  ///
  /// Option<string> y = None;
  /// ASSERT_ANY_THROW(move(y).expect("the world is ending")); // panics with
  ///                                                          // `the world is
  ///                                                          // ending`
  /// ```
  auto expect(std::string_view msg) && -> T {
    if (is_some()) {
      return std::move(value_ref_());
    } else {
      internal::option::expect_value_failed(std::move(msg));
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
  /// ```
  /// Option x = Some("air"s);
  /// ASSERT_EQ(move(x).unwrap(), "air");
  ///
  /// Option<string> y = None;
  /// ASSERT_ANY_THROW(move(y).unwrap());
  /// ```
  auto unwrap() && -> T {
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
  /// ```
  /// ASSERT_EQ(Option(Some("car"s)).unwrap_or("bike"), "car");
  /// ASSERT_EQ(make_none<string>().unwrap_or("bike"), "bike");
  /// ```
  auto unwrap_or(T&& alt) && -> T {
    if (is_some()) {
      return std::move(value_ref_());
    } else {
      return std::move(alt);
    }
  }

  /// Returns the contained value or computes it from a closure.
  ///
  /// # Examples
  ///
  /// ```
  /// int k = 10;
  /// auto alt = [&k]() { return 2 * k; };
  ///
  /// ASSERT_EQ(make_some(4).unwrap_or_else(alt), 4);
  /// ASSERT_EQ(make_none<int>().unwrap_or_else(alt), 20);
  /// ```
  template <NoArgInvocable Fn>
  requires same_as<invoke_result<Fn const&>, T>  //
      auto unwrap_or_else(Fn const& op) && -> T {
    if (is_some()) {
      return std::move(value_ref_());
    } else {
      return op();
    }
  }

  /// Returns the contained value or computes it from a closure.
  ///
  /// # Examples
  ///
  /// ```
  /// int k = 10;
  /// auto alt = [&k]() { return 2 * k; };
  ///
  /// ASSERT_EQ(make_some(4).unwrap_or_else(alt), 4);
  /// ASSERT_EQ(make_none<int>().unwrap_or_else(alt), 20);
  /// ```
  template <NoArgInvocable Fn>
  requires same_as<invoke_result<Fn&>, T>    //
      auto unwrap_or_else(Fn& op) && -> T {  // NOLINT
    if (is_some()) {
      return std::move(value_ref_());
    } else {
      return op();
    }
  }

  /// Maps an `Option<T>` to `Option<U>` by applying a function to a contained
  /// value and therefore, consuming/moving the contained value.
  ///
  /// # Examples
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
  template <OneArgInvocable<T> Fn>
  auto map(Fn const& op) && -> Option<invoke_result<Fn const&, T>> {
    if (is_some()) {
      return Some(op(std::move(value_ref_())));
    } else {
      return None;
    }
  }

  /// Maps an `Option<T>` to `Option<U>` by applying a function to a contained
  /// value and therefore, consuming/moving the contained value.
  ///
  /// # Examples
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
  template <OneArgInvocable<T> Fn>
  auto map(Fn& op) && -> Option<invoke_result<Fn&, T>> {  // NOLINT
    if (is_some()) {
      return Some(op(std::move(value_ref_())));
    } else {
      return None;
    }
  }

  /// Applies a function to the contained value (if any),
  /// or returns the provided alternative (if not).
  ///
  /// # Examples
  ///
  /// ```
  /// Option x = Some("foo"s);
  /// auto alt_fn = [](auto s) { return s.size(); };
  /// ASSERT_EQ(move(x).map_or(alt_fn, 42UL), 3UL);
  ///
  /// Option<string> y = None;
  /// ASSERT_EQ(move(y).map_or(alt_fn, 42UL), 42UL);
  /// ```
  template <OneArgInvocable<T> Fn, InvokeResult<Fn const&, T> A>
  auto map_or(Fn const& op, A&& alt) && -> A {
    if (is_some()) {
      return op(std::move(value_ref_()));
    } else {
      return std::move(alt);
    }
  }

  /// Applies a function to the contained value (if any),
  /// or returns the provided alternative (if not).
  ///
  /// # Examples
  ///
  /// ```
  /// Option x = Some("foo"s);
  /// auto alt_fn = [](auto s) { return s.size(); };
  /// ASSERT_EQ(move(x).map_or(alt_fn, 42UL), 3UL);
  ///
  /// Option<string> y = None;
  /// ASSERT_EQ(move(y).map_or(alt_fn, 42UL), 42UL);
  /// ```
  template <OneArgInvocable<T> Fn, InvokeResult<Fn&, T> A>
  auto map_or(Fn& op, A&& alt) && -> A {  // NOLINT
    if (is_some()) {
      return op(std::move(value_ref_()));
    } else {
      return std::move(alt);
    }
  }

  /// Applies a function to the contained value (if any),
  /// or computes a default (if not).
  ///
  /// # Examples
  ///
  /// ```
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
  template <OneArgInvocable<T> Fn, NoArgInvocable A>
  requires same_as<invoke_result<Fn const&, T>,
                   invoke_result<A const&>>  //
      auto map_or_else(Fn const& op, A const& alt) && {
    if (is_some()) {
      return op(std::move(value_ref_()));
    } else {
      return alt();
    }
  }

  /// Applies a function to the contained value (if any),
  /// or computes a default (if not).
  ///
  /// # Examples
  ///
  /// ```
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
  template <OneArgInvocable<T> Fn, NoArgInvocable A>
  requires same_as<invoke_result<Fn const&, T>,
                   invoke_result<A&>>              //
      auto map_or_else(Fn const& op, A& alt) && {  // NOLINT
    if (is_some()) {
      return op(std::move(value_ref_()));
    } else {
      return alt();
    }
  }

  /// Applies a function to the contained value (if any),
  /// or computes a default (if not).
  ///
  /// # Examples
  ///
  /// ```
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
  template <OneArgInvocable<T> Fn, NoArgInvocable A>
  requires same_as<invoke_result<Fn&, T>,
                   invoke_result<A const&>>        //
      auto map_or_else(Fn& op, A const& alt) && {  // NOLINT
    if (is_some()) {
      return op(std::move(value_ref_()));
    } else {
      return alt();
    }
  }

  /// Applies a function to the contained value (if any),
  /// or computes a default (if not).
  ///
  /// # Examples
  ///
  /// ```
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
  template <OneArgInvocable<T> Fn, NoArgInvocable A>
  requires same_as<invoke_result<Fn&, T>,
                   invoke_result<A&>>        //
      auto map_or_else(Fn& op, A& alt) && {  // NOLINT
    if (is_some()) {
      return op(std::move(value_ref_()));
    } else {
      return alt();
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
  /// ```
  /// Option x = Some("foo"s);
  /// ASSERT_EQ(move(x).ok_or(0), Ok("foo"s));
  ///
  /// Option<string> y = None;
  /// ASSERT_EQ(move(y).ok_or(0), Err(0));
  /// ```
  template <Swappable E>
  auto ok_or(E&& error) && -> Result<T, E> {
    if (is_some()) {
      return Ok(std::move(value_ref_()));
    } else {
      return Err(std::forward<E>(error));
    }
  }

  /// Transforms the `Option<T>` into a `Result<T, E>`, mapping `Some<T>` to
  /// `Ok<T>` and `None` to `Err(op())`.
  ///
  /// # Examples
  ///
  /// ```
  /// auto else_fn = [] () { return 0; };
  ///
  /// Option x = Some("foo"s);
  /// ASSERT_EQ(move(x).ok_or_else(else_fn), Ok("foo"s));
  ///
  /// Option<string> y = None;
  /// ASSERT_EQ(move(y).ok_or_else(else_fn), Err(0));
  /// ```
  template <NoArgInvocable Fn>
  auto ok_or_else(Fn const& op) && -> Result<T, invoke_result<Fn const&>> {
    if (is_some()) {
      return Ok(std::move(value_ref_()));
    } else {
      return Err(op());
    }
  }

  /// Transforms the `Option<T>` into a `Result<T, E>`, mapping `Some<T>` to
  /// `Ok<T>` and `None` to `Err(op())`.
  ///
  /// # Examples
  ///
  /// ```
  /// auto else_fn = [] () { return 0; };
  ///
  /// Option x = Some("foo"s);
  /// ASSERT_EQ(move(x).ok_or_else(else_fn), Ok("foo"s));
  ///
  /// Option<string> y = None;
  /// ASSERT_EQ(move(y).ok_or_else(else_fn), Err(0));
  /// ```
  template <NoArgInvocable Fn>
  auto ok_or_else(Fn& op) && -> Result<T, invoke_result<Fn&>> {  // NOLINT
    if (is_some()) {
      return Ok(std::move(value_ref_()));
    } else {
      return Err(op());
    }
  }

  /// Returns `None` if the option is `None`, otherwise returns `cmp`.
  ///
  /// # Examples
  ///
  /// ```
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
  template <ImplicitlyConstructibleWith<NoneType const&> CmpOption>
  auto AND(CmpOption&& cmp) -> CmpOption {
    if (is_some()) {
      return std::move(cmp);
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
  /// ```
  /// auto sq = [] (auto x) -> Option<int> { return Some(x * x); };
  /// auto nope = [] (auto) -> Option<int> { return None; };
  ///
  /// ASSERT_EQ(make_some(2).and_then(sq).and_then(sq), Some(16));
  /// ASSERT_EQ(make_some(2).and_then(sq).and_then(nope), None);
  /// ASSERT_EQ(make_some(2).and_then(nope).and_then(sq), None);
  /// ASSERT_EQ(make_none<int>().and_then(sq).and_then(sq), None);
  /// ```
  template <OneArgInvocable<T> Fn>
  requires ImplicitlyConstructibleWith<invoke_result<Fn const&, T>,  //
                                       NoneType const&>              //
      auto and_then(Fn const& op) && -> invoke_result<Fn const&, T> {
    if (is_some()) {
      return op(std::move(value_ref_()));
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
  /// ```
  /// auto sq = [] (auto x) -> Option<int> { return Some(x * x); };
  /// auto nope = [] (auto) -> Option<int> { return None; };
  ///
  /// ASSERT_EQ(make_some(2).and_then(sq).and_then(sq), Some(16));
  /// ASSERT_EQ(make_some(2).and_then(sq).and_then(nope), None);
  /// ASSERT_EQ(make_some(2).and_then(nope).and_then(sq), None);
  /// ASSERT_EQ(make_none<int>().and_then(sq).and_then(sq), None);
  /// ```
  template <OneArgInvocable<T> Fn>
  requires ImplicitlyConstructibleWith<invoke_result<Fn&, T>,  //
                                       NoneType const&>        //
      auto and_then(Fn& op) && -> invoke_result<Fn&, T> {      // NOLINT
    if (is_some()) {
      return op(std::move(value_ref_()));
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
  /// ```
  /// auto is_even = [](int n) -> bool { return n % 2 == 0; };
  ///
  /// ASSERT_EQ(make_none<int>().filter(is_even), None);
  /// ASSERT_EQ(make_some(3).filter(is_even), None);
  /// ASSERT_EQ(make_some(4).filter(is_even), Some(4));
  /// ```
  template <OneArgInvocable<T const&> UnaryPredicate>
  requires same_as<invoke_result<UnaryPredicate const&, T const&>,
                   bool>  //
      auto filter(UnaryPredicate const& predicate) && -> Option {
    if (is_some() && predicate(value_cref_())) {
      return std::move(*this);
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
  /// ```
  /// auto is_even = [](int n) -> bool { return n % 2 == 0; };
  ///
  /// ASSERT_EQ(make_none<int>().filter(is_even), None);
  /// ASSERT_EQ(make_some(3).filter(is_even), None);
  /// ASSERT_EQ(make_some(4).filter(is_even), Some(4));
  /// ```
  template <OneArgInvocable<T const&> UnaryPredicate>
  requires same_as<invoke_result<UnaryPredicate&, T const&>, bool>  //
      auto filter(UnaryPredicate& predicate) && -> Option {         // NOLINT
    if (is_some() && predicate(value_cref_())) {
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
  /// ```
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
  auto OR(Option&& alt) && -> Option {
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
  /// ```
  /// auto nobody = []() -> Option<string> { return None; };
  /// auto vikings = []() -> Option<string> { return Some("vikings"s); };
  ///
  /// ASSERT_EQ(Option(Some("barbarians"s)).or_else(vikings),
  /// Some("barbarians"s));
  /// ASSERT_EQ(make_none<string>().or_else(vikings), Some("vikings"s));
  /// ASSERT_EQ(make_none<string>().or_else(nobody), None);
  /// ```
  template <NoArgInvocable Fn>
  requires same_as<invoke_result<Fn const&>, Option>  //
      auto or_else(Fn const& op) && {
    if (is_some()) {
      return std::move(*this);
    } else {
      return op();
    }
  }

  /// Returns the option if it contains a value, otherwise calls `f` and
  /// returns the result.
  ///
  /// # Examples
  ///
  /// ```
  /// auto nobody = []() -> Option<string> { return None; };
  /// auto vikings = []() -> Option<string> { return Some("vikings"s); };
  ///
  /// ASSERT_EQ(Option(Some("barbarians"s)).or_else(vikings),
  /// Some("barbarians"s));
  /// ASSERT_EQ(make_none<string>().or_else(vikings), Some("vikings"s));
  /// ASSERT_EQ(make_none<string>().or_else(nobody), None);
  /// ```
  template <NoArgInvocable Fn>
  requires same_as<invoke_result<Fn&>, Option>  //
      auto or_else(Fn& op) && {                 // NOLINT
    if (is_some()) {
      return std::move(*this);
    } else {
      return op();
    }
  }

  /// Returns whichever one of this object or `alt` is a `Some<T>` variant,
  /// otherwise returns `None` if neither or both are a `Some<T>` variant.
  ///
  ///
  /// # Examples
  ///
  /// ```
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
  auto XOR(Option&& alt) && -> Option {
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
  /// ```
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
  auto take() & -> Option<T> {
    if (is_some()) {
      is_none_ = true;
      return Option<T>(Some<T>(std::move(value_ref_())));
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
  /// ```
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
  auto replace(T&& replacement) & -> Option<T> {
    if (is_some()) {
      std::swap(replacement, value_ref_());
      return Some<T>(std::move(replacement));
    } else {
      construct_some_(Some<T>(std::forward<T>(replacement)));
      is_none_ = false;
      return None;
    }
  }

  auto clone() const -> Option<T> requires CopyConstructible<T> {
    if (is_some()) {
      return Some<T>(T{value_cref_()});
    } else {
      return None;
    }
  }

  /// Unwraps an option, expecting `None` and returning nothing.
  ///
  /// # Panics
  ///
  /// Panics if the value is a `Some`, with a panic message including the
  /// passed message, and the content of the `Some`.
  ///
  ///
  /// # Examples
  ///
  /// ```
  /// auto divide = [](double num, double denom) -> Option<double> {
  /// if (denom == 0.0) return None;
  ///  return Some(num / denom);
  /// };
  ///
  /// EXPECT_ANY_THROW(divide(0.0, 1.0).unwrap_none());
  /// EXPECT_NO_THROW(divide(1.0, 0.0).unwrap_none());
  /// ```
  void expect_none(std::string_view msg) && {
    if (is_some()) {
      internal::option::expect_none_failed(std::move(msg), value_cref_());
    }
    return;
  }

  /// Unwraps an option, expecting `None` and returning nothing.
  ///
  /// # Panics
  ///
  /// Panics if the value is a `Some`, with a panic message including the
  /// passed message, and the content of the `Some`.
  ///
  ///
  /// # Examples
  ///
  /// ```
  /// auto divide = [](double num, double denom) -> Option<double> {
  /// if (denom == 0.0) return None;
  ///  return Some(num / denom);
  /// };
  ///
  /// EXPECT_ANY_THROW(divide(0.0, 1.0).expect_none("zero dividend"));
  /// EXPECT_NO_THROW(divide(1.0, 0.0).expect_none("zero dividend"));
  /// ```
  void unwrap_none() && {
    if (is_some()) {
      internal::option::no_none(value_cref_());
    } else {
      return;
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
  /// ```
  /// Option x = Some("Ten"s);
  /// Option<string> y = None;
  ///
  /// ASSERT_EQ(move(x).unwrap_or_default(), "Ten"s);
  /// ASSERT_EQ(move(y).unwrap_or_default(), ""s);
  /// ```
  auto unwrap_or_default() && -> T requires DefaultConstructible<T> {
    if (is_some()) {
      return std::move(value_ref_());
    } else {
      return T{};
    }
  }

  /// Dereferences the pointer or iterator, therefore returning a const
  /// reference to the pointed-to value (`Option<ConstRef<V>>`).
  ///
  /// Leaves the original Option in-place, creating a new one with a reference
  /// to the original one.
  ///
  /// # Examples
  ///
  /// ```
  /// auto str = "Hello"s;
  /// Option x = Some(&str);
  /// ASSERT_EQ(x.as_const_deref().unwrap().get(), "Hello"s);
  ///
  /// Option<string*> y = None;
  /// ASSERT_EQ(y.as_const_deref(), None);
  /// ```
  auto as_const_deref() const& requires ConstDerefable<T> {
    if (is_some()) {
      return Option<ConstDeref<T>>(Some(ConstDeref<T>{*value_cref_()}));
    } else {
      return Option<ConstDeref<T>>(None);
    }
  }

  [
      [deprecated("calling Result::as_const_deref() on an r-value (temporary), "
                  "therefore binding a "
                  "reference to an object that is about to be destroyed")]] auto
  as_const_deref() const&& requires ConstDerefable<T> {
    if (is_some()) {
      return Option<ConstDeref<T>>(Some(ConstDeref<T>{*value_cref_()}));
    } else {
      return Option<ConstDeref<T>>(None);
    }
  }

  /// Dereferences the pointer or iterator, therefore returning a mutable
  /// reference to the pointed-to value (`Option<MutRef<V>>`).
  ///
  /// Leaves the original `Option` in-place, creating a new one containing a
  /// mutable reference to the inner pointer's dereference value type.
  ///
  /// # Examples
  ///
  /// ```
  /// auto str = "Hello"s;
  /// Option x = Some(&str);
  /// x.as_mut_deref().unwrap().get() = "World"s;
  ///
  /// ASSERT_EQ(str, "World"s);
  ///
  /// Option<string*> z = None;
  /// ASSERT_EQ(z.as_mut_deref(), None);
  /// ```
  auto as_mut_deref() & requires MutDerefable<T> {
    if (is_some()) {
      // the value it points to is not const lets assume the class's state is
      // mutated through the pointer and not make this a const op
      return Option<MutDeref<T>>(Some(MutDeref<T>{*value_ref_()}));
    } else {
      return Option<MutDeref<T>>(None);
    }
  }

  [
      [deprecated("calling Result::as_mut_deref() on an r-value (temporary), "
                  "therefore binding a "
                  "reference to an object that is about to be destroyed")]]  //
      auto
      as_mut_deref() &&
      requires MutDerefable<T> {
    if (is_some()) {
      return Option<MutDeref<T>>(Some(MutDeref<T>{*value_ref_()}));
    } else {
      return Option<MutDeref<T>>(None);
    }
  }

  /// Calls the parameter `some_fn` with the value if this `Option` is a
  /// `Some<T>` variant, else calls `none_fn`. This `Option` is consumed
  /// afterward.
  ///
  /// The return type of both parameters must be same (without expecting
  /// implicit conversions). They can also both return nothing `void`.
  ///
  ///
  /// # Examples
  ///
  /// ```
  /// auto j = make_some("James"s).match([](string name) { return name; },
  ///                                    []() { return "<unidentified>"s; });
  /// ASSERT_EQ(j, "James"s);
  ///
  /// auto k = make_none<string>().match([](string name) { return name; },
  ///                                    []() { return "<unidentified>"s; });
  /// ASSERT_EQ(k, "<unidentified>"s);
  /// ```
  template <OneArgInvocable<T> SomeFn, NoArgInvocable NoneFn>
  requires same_as<invoke_result<SomeFn const&, T>,
                   invoke_result<NoneFn const&>>  //
      auto match(SomeFn const& some_fn, NoneFn const& none_fn) && {
    if (is_some()) {
      return some_fn(std::move(value_ref_()));
    } else {
      return none_fn();
    }
  }

  /// Calls the parameter `some_fn` with the value if this `Option` is a
  /// `Some<T>` variant, else calls `none_fn`. This `Option` is consumed
  /// afterward.
  ///
  /// The return type of both parameters must be same (without expecting
  /// implicit conversions). They can also both return nothing `void`.
  ///
  ///
  /// # Examples
  ///
  /// ```
  /// auto j = make_some("James"s).match([](string name) { return name; },
  ///                                    []() { return "<unidentified>"s; });
  /// ASSERT_EQ(j, "James"s);
  ///
  /// auto k = make_none<string>().match([](string name) { return name; },
  ///                                    []() { return "<unidentified>"s; });
  /// ASSERT_EQ(k, "<unidentified>"s);
  /// ```
  template <OneArgInvocable<T> SomeFn, NoArgInvocable NoneFn>
  requires same_as<invoke_result<SomeFn const&, T>,
                   invoke_result<NoneFn&>>                     //
      auto match(SomeFn const& some_fn, NoneFn& none_fn) && {  // NOLINT
    if (is_some()) {
      return some_fn(std::move(value_ref_()));
    } else {
      return none_fn();
    }
  }

  /// Calls the parameter `some_fn` with the value if this `Option` is a
  /// `Some<T>` variant, else calls `none_fn`. This `Option` is consumed
  /// afterward.
  ///
  /// The return type of both parameters must be same (without expecting
  /// implicit conversions). They can also both return nothing `void`.
  ///
  ///
  /// # Examples
  ///
  /// ```
  /// auto j = make_some("James"s).match([](string name) { return name; },
  ///                                    []() { return "<unidentified>"s; });
  /// ASSERT_EQ(j, "James"s);
  ///
  /// auto k = make_none<string>().match([](string name) { return name; },
  ///                                    []() { return "<unidentified>"s; });
  /// ASSERT_EQ(k, "<unidentified>"s);
  /// ```
  template <OneArgInvocable<T> SomeFn, NoArgInvocable NoneFn>
  requires same_as<invoke_result<SomeFn&, T>,
                   invoke_result<NoneFn const&>>               //
      auto match(SomeFn& some_fn, NoneFn const& none_fn) && {  // NOLINT
    if (is_some()) {
      return some_fn(std::move(value_ref_()));
    } else {
      return none_fn();
    }
  }

  /// Calls the parameter `some_fn` with the value if this `Option` is a
  /// `Some<T>` variant, else calls `none_fn`. This `Option` is consumed
  /// afterward.
  ///
  /// The return type of both parameters must be same (without expecting
  /// implicit conversions). They can also both return nothing `void`.
  ///
  ///
  /// # Examples
  ///
  /// ```
  /// auto j = make_some("James"s).match([](string name) { return name; },
  ///                                    []() { return "<unidentified>"s; });
  /// ASSERT_EQ(j, "James"s);
  ///
  /// auto k = make_none<string>().match([](string name) { return name; },
  ///                                    []() { return "<unidentified>"s; });
  /// ASSERT_EQ(k, "<unidentified>"s);
  /// ```
  template <OneArgInvocable<T> SomeFn, NoArgInvocable NoneFn>
  requires same_as<invoke_result<SomeFn&, T>,
                   invoke_result<NoneFn&>>               //
      auto match(SomeFn& some_fn, NoneFn& none_fn) && {  // NOLINT
    if (is_some()) {
      return some_fn(std::move(value_ref_()));
    } else {
      return none_fn();
    }
  }

 private:
  storage_type storage_;
  bool is_none_;

  // returns a copy of the contained value
  void construct_some_(Some<T>&& value) {
    //
    // object is created here with a placement
    // we always call std::move or the destructor so the object is moved and
    // destructors can be called through the moved object and thus leave this
    // buffer representation in an unspecified state
    auto buffer_ptr = &storage_.get_ref();
    new (buffer_ptr) Some<T>{std::forward<Some<T>>(value)};
  }

  Some<T> const& value_wrap_cref_() const {
    auto buffer = reinterpret_cast<Some<T> const*>(storage_.get());
    return *std::launder(buffer);
  }

  Some<T>& value_wrap_ref_() {
    auto buffer = reinterpret_cast<Some<T>*>(storage_.get());
    return *std::launder(buffer);
  }

  T& value_ref_() { return value_wrap_ref_().value_; }

  T const& value_cref_() const { return value_wrap_cref_().value_; }
};

//! [section="Result"]
//! Error handling with the `Result` type.
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
//! ```
//! enum class Version { Version1 = 1, Version2 = 2 };
//!
//! auto parse_version =
//!     [](array<uint8_t, 5> const& header) -> Result<Version, string_view> {
//!   if (header.size() < 1) {
//!     return Err("invalid header length"sv);
//!   } else if (header[0] == 1) {
//!     return Ok(Version::Version1);
//!   } else if (header[0] == 2) {
//!     return Ok(Version::Version2);
//!   } else {
//!     return Err("invalid version"sv);
//!   }
//! };
//!
//! parse_version({0x01u, 0x02u, 0x03u, 0x04u, 0x05u})
//!     .match(
//!         [](auto value) { fmt::print("Working with version: {}\n", value);
//!         },
//!         [](auto err) { fmt::print("Error parsing header: {}\n", err); });
//! ```
//!
//!
//! `Result` comes with some convenience methods that make working with it more
//! succinct.
//!
//! ```
//! Result<int, int> good_result = Ok(10);
//! Result<int, int> bad_result = Err(10);
//!
//! // The `is_ok` and `is_err` methods do what they say.
//! ASSERT_TRUE(good_result.is_ok() && !good_result.is_err());
//! ASSERT_TRUE(bad_result.is_err() && !bad_result.is_ok());
//! ```
//!
/// `Result` is a type that represents either success (`Ok`) or failure (`Err`).
///
/// Result is either in the Ok or Err state at any point in time
template <Swappable T, Swappable E>
class Result {
 public:
  static_assert(!std::is_reference_v<T>,
                "Cannot use T& for type, To prevent subtleties use "
                "std::reference_wrapper<T> instead");

  using value_type = T;
  using error_type = E;
  using storage_type = pair_variant_storage<Ok<T>, Err<E>>;

  Result() = delete;

  Result(Ok<T>&& result) {  // NOLINT
    construct_value_(std::forward<Ok<T>>(result));
    is_ok_ = true;
  }

  Result(Err<E>&& err) {  // NOLINT
    construct_err_(std::forward<Err<E>>(err));
    is_ok_ = false;
  }

  Result(Result const& rhs) = default;
  Result& operator=(Result const& rhs) = default;

  Result(Result&& rhs) {
    if (rhs.is_ok()) {
      construct_value_(std::move(rhs.value_wrap_ref_()));
    } else {
      construct_err_(std::move(rhs.err_wrap_ref_()));
    }
    is_ok_ = std::move(rhs.is_ok_);
  }

  Result& operator=(Result&& rhs) {
    std::swap(is_ok_, rhs.is_ok_);
    std::swap(value_wrap_ref_(), rhs.value_wrap_ref_());
    return *this;
  }

  ~Result() noexcept {
    if (is_ok()) {
      value_wrap_ref_().~Ok<T>();
    } else {
      err_wrap_ref_().~Err<E>();
    }
  };

  bool operator==(Ok<T> const& cmp) const requires EqualityComparable<T> {
    if (is_ok()) {
      return value_wrap_cref_() == cmp;
    } else {
      return false;
    }
  }

  bool operator==(Ok<ConstRef<T>> const& cmp) const
      requires EqualityComparable<T> {
    if (is_ok()) {
      return value_wrap_cref_() == cmp;
    } else {
      return false;
    }
  }

  bool operator==(Ok<MutRef<T>> const& cmp) const
      requires EqualityComparable<T> {
    if (is_ok()) {
      return value_wrap_cref_() == cmp;
    } else {
      return false;
    }
  }

  bool operator==(Ok<T const*> const& cmp) const
      requires EqualityComparable<T> {
    if (is_ok()) {
      return value_wrap_cref_() == cmp;
    } else {
      return false;
    }
  }

  bool operator==(Ok<T*> const& cmp) const requires EqualityComparable<T> {
    if (is_ok()) {
      return value_wrap_cref_() == cmp;
    } else {
      return false;
    }
  }

  bool operator==(Err<E> const& cmp) const requires EqualityComparable<E> {
    if (is_ok()) {
      return false;
    } else {
      return err_wrap_cref_() == cmp;
    }
  }

  bool operator==(Err<ConstRef<E>> const& cmp) const
      requires EqualityComparable<E> {
    if (is_ok()) {
      return false;
    } else {
      return err_wrap_cref_() == cmp;
    }
  }

  bool operator==(Err<MutRef<E>> const& cmp) const
      requires EqualityComparable<E> {
    if (is_ok()) {
      return false;
    } else {
      return err_wrap_cref_() == cmp;
    }
  }

  bool operator==(Err<E const*> const& cmp) const
      requires EqualityComparable<E> {
    if (is_ok()) {
      return false;
    } else {
      return err_wrap_cref_() == cmp;
    }
  }

  bool operator==(Err<E*> const& cmp) const requires EqualityComparable<E> {
    if (is_ok()) {
      return false;
    } else {
      return err_wrap_cref_() == cmp;
    }
  }

  bool operator==(Result const& cmp) const
      requires EqualityComparable<T>&& EqualityComparable<E> {
    if (is_ok() && cmp.is_ok()) {
      return value_wrap_cref_() == cmp.value_wrap_cref_();
    } else if (is_err() && cmp.is_err()) {
      return err_wrap_cref_() == cmp.err_wrap_cref_();
    } else {
      return false;
    }
  }

  /// Returns `true` if the result is an `Ok<T>` variant.
  ///
  ///
  /// # Examples
  ///
  /// ```
  /// Result<int, string_view> x = Ok(-3);
  /// ASSERT_TRUE(x.is_ok());
  ///
  /// Result<int, string_view> y = Err("Some error message"sv);
  /// ASSERT_FALSE(y.is_ok());
  /// ```
  bool is_ok() const noexcept { return is_ok_; }

  /// Returns `true` if the result is `Err<T>`.
  ///
  /// # Examples
  ///
  /// ```
  /// Result<int, string_view> x = Ok(-3);
  /// ASSERT_FALSE(x.is_err());
  ///
  /// Result<int, string_view> y = Err("Some error message"sv);
  /// ASSERT_TRUE(y.is_err());
  /// ```
  bool is_err() const noexcept { return !is_ok(); }

  /// Returns `true` if the result is an `Ok<T>` variant and contains the given
  /// value.
  ///
  /// # Examples
  ///
  /// ```
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
  template <EqualityComparable<T const&> CmpType>
  bool contains(CmpType const& cmp) const {
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
  /// ```
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
  template <EqualityComparable<E const&> ErrCmp>
  bool contains_err(ErrCmp const& cmp) const {
    if (is_ok()) {
      return false;
    } else {
      return err_cref_() == cmp;
    }
  }

  /// Converts from `Result<T, E>` to `Option<T>`.
  ///
  /// Converts this result into an `Option<T>`, consuming itself,
  /// and discarding the error, if any.
  ///
  /// # Examples
  ///
  /// ```
  /// Result<int, string> x = Ok(2);
  /// ASSERT_EQ(move(x).ok(), Some(2));
  ///
  /// Result<int, string> y = Err("Nothing here"s);
  /// ASSERT_EQ(move(y).ok(), None);
  /// ```
  auto ok() && -> Option<T> {
    if (is_ok()) {
      // value to be left in an unspecified state and destroyed as appropriate
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
  /// ```
  /// Result<int, string> x = Ok(2);
  /// ASSERT_EQ(move(x).err(), None);
  ///
  /// Result<int, string> y = Err("Nothing here"s);
  /// ASSERT_EQ(move(y).err(), Some("Nothing here"s));
  /// ```
  auto err() && -> Option<E> {
    if (is_ok()) {
      // value to be left in an unspecified state and destroyed as appropriate
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
  /// ```
  /// Result<int, string> x = Ok(2);
  /// ASSERT_EQ(x.as_const_ref().unwrap().get(), 2);
  ///
  /// Result<int, string> y = Err("Error"s);
  /// ASSERT_EQ(y.as_const_ref().unwrap_err().get(), "Error"s);
  /// ```
  auto as_const_ref() const& -> Result<ConstRef<T>, ConstRef<E>> {
    if (is_ok()) {
      return Ok(ConstRef<T>(value_cref_()));
    } else {
      return Err(ConstRef<E>(err_cref_()));
    }
  }

  [
      [deprecated("calling Result::as_const_ref() on an r-value (temporary), "
                  "therefore binding a reference to an object that is about to "
                  "be destroyed")]]  //
  auto
  as_const_ref() const&& -> Result<ConstRef<T>, ConstRef<E>> {
    if (is_ok()) {
      return Ok(ConstRef<T>(value_cref_()));
    } else {
      return Err(ConstRef<E>(err_cref_()));
    }
  }

  /// Converts from `Result<T, E> &` to `Result<MutRef<T>, MutRef<E>>`.
  ///
  /// # Examples
  ///
  /// ```
  /// auto mutate = [](Result<int, int>& r) {
  ///  r.as_mut_ref().match([](auto ok) { ok.get() = 42; },
  ///                       [](auto err) { err.get() = 0; });
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
  auto as_mut_ref() & -> Result<MutRef<T>, MutRef<E>> {
    if (is_ok()) {
      return Ok(MutRef<T>(value_ref_()));
    } else {
      return Err(MutRef<E>(err_ref_()));
    }
  }

  [
      [deprecated("calling Result::as_mut_ref() on an r-value (temporary), "
                  "therefore binding a "
                  "reference to an object that is about to be destroyed")]]  //
  auto
  as_mut_ref() && -> Result<MutRef<T>, MutRef<E>> {
    if (is_ok()) {
      return Ok(MutRef<T>(value_ref_()));
    } else {
      return Err(MutRef<E>(err_ref_()));
    }
  }

  /// Maps a `Result<T, E>` to `Result<U, E>` by applying the function `op` to
  /// the contained `Ok<T>` value, leaving an `Err<E>` value untouched.
  ///
  /// This function can be used to compose the results of two functions.
  ///
  /// # Examples
  ///
  /// Extract the content-type from an http header
  ///
  /// ```
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
  template <OneArgInvocable<T> Fn>
  auto map(Fn const& op) && -> Result<invoke_result<Fn const&, T>, E> {
    if (is_ok()) {
      return Ok(op(std::move(value_ref_())));
    } else {
      return Err(std::move(err_ref_()));
    }
  }

  /// Maps a `Result<T, E>` to `Result<U, E>` by applying the function `op` to
  /// the contained `Ok<T>` value, leaving an `Err<E>` value untouched.
  ///
  /// This function can be used to compose the results of two functions.
  ///
  /// # Examples
  ///
  /// Extract the content-type from an http header
  ///
  /// ```
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
  template <OneArgInvocable<T> Fn>
  auto map(Fn& op) && -> Result<invoke_result<Fn&, T>, E> {  // NOLINT
    if (is_ok()) {
      return Ok(op(std::move(value_ref_())));
    } else {
      return Err(std::move(err_ref_()));
    }
  }

  /// Applies a function to the contained value (if any),
  /// or returns the provided default (if not).
  ///
  /// # Examples
  ///
  /// ```
  /// Result<string, int> x = Ok("foo"s);
  /// auto map_fn = [](auto s) { return s.size(); };
  /// ASSERT_EQ(move(x).map_or(map_fn, 42UL), 3UL);
  ///
  /// Result<string, int> y = Err(-404);
  /// ASSERT_EQ(move(y).map_or(map_fn, 42UL), 42UL);
  /// ```
  template <OneArgInvocable<T> Fn, InvokeResult<Fn const&, T> ReturnType>
  requires same_as<invoke_result<Fn const&, T>, ReturnType>  //
      auto map_or(Fn const& op, ReturnType&& alt) && -> ReturnType {
    if (is_ok()) {
      return op(std::move(value_ref_()));
    } else {
      return std::move(alt);
    }
  }

  /// Applies a function to the contained value (if any),
  /// or returns the provided default (if not).
  ///
  /// # Examples
  ///
  /// ```
  /// Result<string, int> x = Ok("foo"s);
  /// auto map_fn = [](auto s) { return s.size(); };
  /// ASSERT_EQ(move(x).map_or(map_fn, 42UL), 3UL);
  ///
  /// Result<string, int> y = Err(-404);
  /// ASSERT_EQ(move(y).map_or(map_fn, 42UL), 42UL);
  /// ```
  template <OneArgInvocable<T> Fn, InvokeResult<Fn&, T> ReturnType>
  requires same_as<invoke_result<Fn&, T>, ReturnType>   //
      auto map_or(Fn& op,                               // NOLINT
                  ReturnType&& alt) && -> ReturnType {  // NOLINT
    if (is_ok()) {
      return op(std::move(value_ref_()));
    } else {
      return std::move(alt);
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
  /// ```
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
  template <OneArgInvocable<T> Fn, OneArgInvocable<E> A>
  requires same_as<invoke_result<Fn const&, T>, invoke_result<A const&, E>>  //
      auto map_or_else(Fn const& op, A const& alt_op) && {  // NOLINT
    if (is_ok()) {
      return op(std::move(value_ref_()));
    } else {
      return alt_op(std::move(err_ref_()));
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
  /// ```
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
  template <OneArgInvocable<T> Fn, OneArgInvocable<E> A>
  requires same_as<invoke_result<Fn const&, T>, invoke_result<A&, E>>  //
      auto map_or_else(Fn const& op, A& alt_op) && {                   // NOLINT
    if (is_ok()) {
      return op(std::move(value_ref_()));
    } else {
      return alt_op(std::move(err_ref_()));
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
  /// ```
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
  template <OneArgInvocable<T> Fn, OneArgInvocable<E> A>
  requires same_as<invoke_result<Fn&, T>, invoke_result<A const&, E>>  //
      auto map_or_else(Fn& op, A const& alt_op) && {                   // NOLINT
    if (is_ok()) {
      return op(std::move(value_ref_()));
    } else {
      return alt_op(std::move(err_ref_()));
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
  /// ```
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
  template <OneArgInvocable<T> Fn, OneArgInvocable<E> A>
  requires same_as<invoke_result<Fn&, T>, invoke_result<A&, E>>  //
      auto map_or_else(Fn& op, A& alt_op) && {                   // NOLINT
    if (is_ok()) {
      return op(std::move(value_ref_()));
    } else {
      return alt_op(std::move(err_ref_()));
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
  /// ```
  /// auto stringify = [](auto x) { return "error code: " + std::to_string(x);
  /// };
  ///
  /// Result<int, int> x = Ok(2);
  /// ASSERT_EQ(move(x).map_err(stringify), Ok(2));
  ///
  /// Result<int, int> y = Err(404);
  /// ASSERT_EQ(move(y).map_err(stringify), Err("error code: 404"s));
  /// ```
  template <OneArgInvocable<E> Fn>
  auto map_err(Fn const& op) && -> Result<T, invoke_result<Fn const&, E>> {
    if (is_ok()) {
      return Ok(std::move(value_ref_()));
    } else {
      return Err(op(std::move(err_ref_())));
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
  /// ```
  /// auto stringify = [](auto x) { return "error code: " + std::to_string(x);
  /// };
  ///
  /// Result<int, int> x = Ok(2);
  /// ASSERT_EQ(move(x).map_err(stringify), Ok(2));
  ///
  /// Result<int, int> y = Err(404);
  /// ASSERT_EQ(move(y).map_err(stringify), Err("error code: 404"s));
  /// ```
  template <OneArgInvocable<E> Fn>
  auto map_err(Fn& op) && -> Result<T, invoke_result<Fn&, E>> {  // NOLINT
    if (is_ok()) {
      return Ok(std::move(value_ref_()));
    } else {
      return Err(op(std::move(err_ref_())));
    }
  }

  /// Returns `res` if the result is `Ok`, otherwise returns the `Err` value
  /// of itself.
  ///
  /// # Examples
  ///
  /// ```
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
  template <ImplicitlyConstructibleWith<Err<E>> CmpResult>
  auto AND(CmpResult&& res) && -> CmpResult {
    if (is_ok()) {
      return std::move(res);
    } else {
      return Err(std::move(err_ref_()));
    }
  }

  /// Calls `op` if the result is `Ok`, otherwise returns the `Err` value of
  /// itself.
  ///
  /// This function can be used for control flow based on `Result` values.
  ///
  /// # Examples
  ///
  /// ```
  /// auto sq = [](int x) { return x * x; };
  ///
  /// auto make_ok = [](int x) -> Result<int, int> { return Ok(move(x)); };
  /// auto make_err = [](int x) -> Result<int, int> { return Err(move(x)); };
  ///
  /// ASSERT_EQ(make_ok(2).and_then(sq).and_then(sq), Ok(16));
  /// ASSERT_EQ(make_err(3).and_then(sq).and_then(sq), Err(3));
  /// ```
  template <OneArgInvocable<T> Fn>
  auto and_then(Fn const& op) && -> Result<invoke_result<Fn const&, T>, E> {
    if (is_ok()) {
      return Ok(op(std::move(value_ref_())));
    } else {
      return Err(std::move(err_ref_()));
    }
  }

  /// Calls `op` if the result is `Ok`, otherwise returns the `Err` value of
  /// itself.
  ///
  /// This function can be used for control flow based on `Result` values.
  ///
  /// # Examples
  ///
  /// ```
  /// auto sq = [](int x) { return x * x; };
  ///
  /// auto make_ok = [](int x) -> Result<int, int> { return Ok(move(x)); };
  /// auto make_err = [](int x) -> Result<int, int> { return Err(move(x)); };
  ///
  /// ASSERT_EQ(make_ok(2).and_then(sq).and_then(sq), Ok(16));
  /// ASSERT_EQ(make_err(3).and_then(sq).and_then(sq), Err(3));
  /// ```
  template <OneArgInvocable<T> Fn>
  auto and_then(Fn& op) && -> Result<invoke_result<Fn&, T>, E> {  // NOLINT
    if (is_ok()) {
      return Ok(op(std::move(value_ref_())));
    } else {
      return Err(std::move(err_ref_()));
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
  /// ```
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
  template <ImplicitlyConstructibleWith<Ok<T>> AltResult>
  auto OR(AltResult&& alt) && -> AltResult {
    if (is_ok()) {
      return Ok(std::move(value_ref_()));
    } else {
      return std::move(alt);
    }
  }

  /// Calls `op` if the result is `Err`, otherwise returns the `Ok` value of
  /// itself.
  ///
  /// This function can be used for control flow based on result values.
  ///
  /// # Examples
  ///
  /// ```
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
  template <OneArgInvocable<E> Fn>
  requires ImplicitlyConstructibleWith<invoke_result<Fn const&, E>,
                                       Ok<T>>  //
      auto or_else(Fn const& op) && -> invoke_result<Fn const&, E> {
    if (is_ok()) {
      return Ok(std::move(value_ref_()));
    } else {
      return op(std::move(err_ref_()));
    }
  }

  /// Calls `op` if the result is `Err`, otherwise returns the `Ok` value of
  /// itself.
  ///
  /// This function can be used for control flow based on result values.
  ///
  /// # Examples
  ///
  /// ```
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
  template <OneArgInvocable<E> Fn>
  requires ImplicitlyConstructibleWith<invoke_result<Fn&, E>,
                                       Ok<T>>             //
      auto or_else(Fn& op) && -> invoke_result<Fn&, E> {  // NOLINT
    if (is_ok()) {
      return Ok(std::move(value_ref_()));
    } else {
      return op(std::move(err_ref_()));
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
  /// ```
  /// int alt = 2;
  /// Result<int, string_view> x = Ok(9);
  /// ASSERT_EQ(move(x).unwrap_or(move(alt)), 9);
  ///
  /// int alt_b = 2;
  /// Result<int, string_view> y = Err("error"sv);
  /// ASSERT_EQ(move(y).unwrap_or(move(alt_b)), 2);
  /// ```
  auto unwrap_or(T&& alt) && -> T {
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
  /// ```
  /// auto count = [] (string_view err)  { return err.size(); };
  ///
  /// ASSERT_EQ(make_ok<size_t,string_view>(2UL).unwrap_or_else(count), 2);
  /// ASSERT_EQ(make_err<size_t,string_view>("booo"sv).unwrap_or_else(count),
  /// 4);
  /// ```
  template <OneArgInvocable<E> Fn>
  requires same_as<invoke_result<Fn const&, E>, T>  //
      auto unwrap_or_else(Fn const& op) && -> T {
    if (is_ok()) {
      return std::move(value_ref_());
    } else {
      return op(std::move(err_ref_()));
    }
  }

  /// Unwraps a result, yielding the content of an `Ok`.
  /// If the value is an `Err` then it calls `op` with its value.
  ///
  ///
  /// # Examples
  ///
  /// ```
  /// auto count = [] (string_view err)  { return err.size(); };
  ///
  /// ASSERT_EQ(make_ok<size_t,string_view>(2UL).unwrap_or_else(count), 2);
  /// ASSERT_EQ(make_err<size_t,string_view>("booo"sv).unwrap_or_else(count),
  /// 4);
  /// ```
  template <OneArgInvocable<E> Fn>
  requires same_as<invoke_result<Fn&, E>, T>  //
      auto unwrap_or_else(Fn& op) && -> T {   // NOLINT
    if (is_ok()) {
      return std::move(value_ref_());
    } else {
      return op(std::move(err_ref_()));
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
  /// ```
  /// ASSERT_EQ(make_ok<int, string_view>(2).unwrap(), 2);
  /// Result<int, string_view> x = Err("emergency failure"sv);
  /// ASSERT_ANY_THROW(move(x).unwrap());
  /// ```
  auto unwrap() && -> T {
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
  /// ```
  /// Result<int, string_view> x = Err("emergency failure"sv);
  /// ASSERT_ANY_THROW(move(x).expect("Testing expect"));
  /// ```
  auto expect(std::string_view msg) && -> T {
    if (is_err()) {
      internal::result::expect_value_failed(std::move(msg), err_cref_());
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
  /// ```
  /// Result<int, string_view> x = Ok(2);
  /// ASSERT_ANY_THROW(move(x).unwrap_err()); // panics
  ///
  /// Result<int, string_view> y = Err("emergency failure"sv);
  /// ASSERT_EQ(move(y).unwrap_err(), "emergency failure");
  /// ```
  auto unwrap_err() && -> E {
    if (is_ok()) {
      internal::result::no_error(value_cref_());
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
  /// ```
  /// Result<int, string_view> x = Ok(10);
  /// ASSERT_ANY_THROW(move(x).expect_err("Testing expect_err")); // panics with
  ///                                                             // `Testing
  ///                                                             // expect_err:
  ///                                                             10`
  /// ```
  auto expect_err(std::string_view msg) && -> E {
    if (is_ok()) {
      internal::result::expect_err_failed(std::move(msg), value_cref_());
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
  /// ```
  /// Result<string, int> good_year = Ok("1909"s);
  /// Result<string, int> bad_year = Err(-1);
  ///
  /// ASSERT_EQ(move(good_year).unwrap_or_default(), "1909"s);
  /// ASSERT_EQ(move(bad_year).unwrap_or_default(), ""s); // empty string (""s)
  ///                                                     // is the default
  ///                                                     // value
  ///                                                     // for a C++ string
  /// ```
  auto unwrap_or_default() && -> T requires DefaultConstructible<T> {
    if (is_ok()) {
      return std::move(value_ref_());
    } else {
      return T{};
    }
  }

  /// Performs a constant dereference on `T`. if `T` is a pointer, C++-style
  /// iterator or smart-pointer. i.e. Converts `Result<T, E>` to
  /// `Result<ConstRef<U>, ConstRef<E>>`
  /// Where:
  /// -  `U` represents the value obtained from dereferencing `T` and
  /// `ConstRef<U>` is a constant reference to a value pointed to by the value's
  /// pointer-type `T`
  ///
  ///
  /// NOTE: `ConstRef<U>` is an alias for `std::reference_wrapper<U const>`, but
  /// that's too long :)
  /// NOTE: If `T` is an owning pointer/iterator/object, This result
  /// should live just as long as the dereference result obtained by calling
  /// this method.
  ///
  ///
  /// # Examples
  ///
  /// ```
  /// int v = 98;
  ///
  /// Result<int*, string_view> x = Ok(&v);
  /// ConstRef<int> v_ref = x.as_const_deref().unwrap();
  ///
  /// ASSERT_EQ(v_ref.get(), 98);    // check the values
  /// ASSERT_EQ(&v_ref.get(), &v);  // check their addresses
  ///
  /// Result<int*, string_view> y = Err("Errrr...."sv);
  /// ConstRef<string_view> y_err_ref = y.as_const_deref().unwrap_err();
  /// ASSERT_EQ(y_err_ref.get(), "Errrr...."sv);  // check the string-view's
  ///                                             // value
  /// ```
  auto as_const_deref() const& requires ConstDerefable<T> {
    using result_t = Result<ConstDeref<T>, ConstRef<E>>;

    if (is_ok()) {
      return result_t(Ok(ConstDeref<T>{*value_cref_()}));
    } else {
      return result_t(Err(ConstRef<E>(err_cref_())));
    }
  }

  [
      [deprecated("calling Result::as_const_deref() on an r-value (temporary), "
                  "therefore binding a "
                  "reference to an object that is about to be destroyed")]]  //
  auto
  as_const_deref() const&& requires ConstDerefable<T> {
    using result_t = Result<ConstDeref<T>, ConstRef<E>>;

    if (is_ok()) {
      return result_t(Ok(ConstDeref<T>{*value_cref_()}));
    } else {
      return result_t(Err(ConstRef<E>(err_cref_())));
    }
  }

  /// Performs a constant dereference on `E`. if `E` is a pointer, C++-style
  /// iterator or smart-pointer. i.e. Converts `Result<T, E>` to
  /// `Result<ConstRef<T>, ConstRef<F>>`
  /// Where:
  /// -  `F` represents the value obtained from dereferencing `E` and
  /// `ConstRef<F>` is a constant reference to a value pointed to by the error's
  /// pointer-type `E`
  ///
  ///
  /// NOTE: `ConstRef<F>` is an alias for `std::reference_wrapper<F const>`, but
  /// that's too long :)
  /// NOTE: If `E` is an owning pointer/iterator/object, This result
  /// should live just as long as the dereference result obtained by calling
  /// this method.
  ///
  ///
  /// # Examples
  ///
  /// ```
  /// Result<int, string_view*> x = Ok(2);
  /// ConstRef<int> x_value_ref = x.as_const_deref_err().unwrap();
  /// ASSERT_EQ(x_value_ref.get(), 2);  // check their values
  ///
  /// string_view e = "Errrr...."sv;
  ///
  /// Result<int, string_view*> y = Err(&e);
  /// ConstRef<string_view> y_err_ref = y.as_const_deref_err().unwrap_err();
  ///
  /// ASSERT_EQ(y_err_ref.get(), e);    // check their values
  /// ASSERT_EQ(&y_err_ref.get(), &e);  // check their addresses
  /// ```
  auto as_const_deref_err() const& requires ConstDerefable<E> {
    using result_t = Result<ConstRef<T>, ConstDeref<E>>;

    if (is_ok()) {
      return result_t(Ok(ConstRef<T>{value_cref_()}));
    } else {
      return result_t(Err(ConstDeref<E>{*err_cref_()}));
    }
  }

  [[deprecated(
      "calling Result::as_const_deref_err() on an r-value (temporary), "
      "therefore binding a "
      "reference to an object that is about to be destroyed")]]  //
  auto
  as_const_deref_err() const&& requires ConstDerefable<E> {
    using result_t = Result<ConstRef<T>, ConstDeref<E>>;

    if (is_ok()) {
      return result_t(Ok(ConstRef<T>{value_cref_()}));
    } else {
      return result_t(Err(ConstDeref<E>{*err_cref_()}));
    }
  }

  /// Performs a mutable dereference on `T`, if `T` is a pointer, C++-style
  /// iterator or smart-pointer. i.e. Converts `Result<T, E>` to
  /// `Result<MutRef<U>, MutRef<E>>`
  /// Where:
  /// -  `U` represents the value obtained from dereferencing `T` and
  /// `MutRef<U>` is a mutable reference to a value pointed to by the value's
  /// pointer-type `T`
  ///
  ///
  /// NOTE: `MutRef<U>` is an alias for std::reference_wrapper<U>, but
  /// that's too long :)
  /// NOTE: If `T` is an owning pointer/iterator/object, This result
  /// should live just as long as the dereference result obtained by calling
  /// this method.
  ///
  ///
  /// # Examples
  ///
  /// ```
  /// int v = 98;
  ///
  /// Result<int*, string_view> x = Ok(&v);
  /// MutRef<int> v_ref = x.as_mut_deref().unwrap();
  ///
  /// ASSERT_EQ(v_ref.get(), 98);   // check the values
  /// ASSERT_EQ(&v_ref.get(), &v);  // check their addresses
  ///
  /// v_ref.get() = -404;  // change v's value via the mutable reference
  ///
  /// ASSERT_EQ(v_ref.get(), -404);  // check that the reference's value changed
  /// ASSERT_EQ(v, -404);            // check that v's value changed
  /// ASSERT_EQ(v_ref.get(), v);     // check that both v and v_ref are equal
  /// ASSERT_EQ(&v_ref.get(), &v);   // check that v_ref references v
  ///
  /// Result<int*, string_view> y = Err("Errrr...."sv);
  /// MutRef<string_view> y_err_ref = y.as_mut_deref().unwrap_err();
  /// ASSERT_EQ(y_err_ref.get(), "Errrr...."sv);
  ///
  /// y_err_ref.get() = "Omoshiroi!..."sv;  // change the error's value
  ///
  /// ASSERT_EQ(y, Err("Omoshiroi!..."sv));  // check that the error's value was
  ///                                        // actually changed
  /// ```
  auto as_mut_deref() & requires MutDerefable<T> {
    using result_t = Result<MutDeref<T>, MutRef<E>>;
    if (is_ok()) {
      return result_t(Ok(MutDeref<T>{*value_ref_()}));
    } else {
      return result_t(Err(MutRef<E>(err_ref_())));
    }
  }

  [
      [deprecated("calling Result::as_mut_deref() on an r-value (temporary), "
                  "therefore binding a "
                  "reference to an object that is about to be destroyed")]]  //
      auto
      as_mut_deref() &&
      requires MutDerefable<T> {
    using result_t = Result<MutDeref<T>, MutRef<E>>;
    if (is_ok()) {
      return result_t(Ok(MutDeref<T>{*value_ref_()}));
    } else {
      return result_t(Err(MutRef<E>(err_ref_())));
    }
  }

  /// Performs a mutable dereference on `E`, if `E` is a pointer, C++-style
  /// iterator or smart-pointer. i.e. Converts `Result<T, E>` to
  /// `Result<MutRef<T>, MutRef<F>>`
  /// Where:
  /// -  `F` represents the value obtained from dereferencing `E` and
  /// `MutRef<F>` is a mutable reference to a value pointed to by the value's
  /// pointer-type `T`
  ///
  ///
  /// NOTE: `MutRef<F>` is an alias for std::reference_wrapper<F>, but
  /// that's too long :)
  /// NOTE: If `E` is an owning pointer/iterator/object, This result
  /// should live just as long as the dereference result obtained by calling
  /// this method.
  ///
  ///
  /// # Examples
  ///
  /// ```
  /// int e = 98;
  ///
  /// Result<string_view, int*> x = Err(&e);
  /// MutRef<int> e_ref = x.as_mut_deref_err().unwrap_err();
  ///
  /// ASSERT_EQ(e_ref.get(), 98);   // check the values
  /// ASSERT_EQ(&e_ref.get(), &e);  // check their addresses
  ///
  /// e_ref.get() = -404;  // change e's value via the mutable reference
  ///
  /// ASSERT_EQ(e_ref.get(), -404);  // check that the reference's value changed
  /// ASSERT_EQ(e, -404);            // check that v's value changed
  /// ASSERT_EQ(e_ref.get(), e);     // check that both v and v_ref are equal
  /// ASSERT_EQ(&e_ref.get(), &e);   // check that v_ref references v
  ///
  /// Result<string_view, int*> y = Ok("Errrr...."sv);
  /// MutRef<string_view> y_err_ref = y.as_mut_deref_err().unwrap();
  /// ASSERT_EQ(y_err_ref.get(), "Errrr...."sv);
  ///
  /// y_err_ref.get() = "Omoshiroi!..."sv;  // change the error's value
  ///
  /// ASSERT_EQ(y, Ok("Omoshiroi!..."sv));  // check that the error's value was
  ///                                       // actually changed
  /// ```
  auto as_mut_deref_err() & requires MutDerefable<E> {
    using result_t = Result<MutRef<T>, MutDeref<E>>;
    if (is_ok()) {
      return result_t(Ok(MutRef<T>{value_ref_()}));
    } else {
      return result_t(Err(MutDeref<E>(*err_ref_())));
    }
  }

  [[deprecated(
      "calling Result::as_mut_deref_err() on an r-value (temporary), "
      "therefore binding a "
      "reference to an object that is about to be destroyed")]]  //
      auto
      as_mut_deref_err() &&
      requires MutDerefable<E> {
    using result_t = Result<MutRef<T>, MutDeref<E>>;
    if (is_ok()) {
      return result_t(Ok(MutRef<T>{value_ref_()}));
    } else {
      return result_t(Err(MutDeref<E>(*err_ref_())));
    }
  }

  /// Calls the parameter `ok_fn` with the value if this result is an `Ok<T>`,
  /// else calls `err_fn` with the error. This result is consumed afterward.
  ///
  /// The return type of both parameters must be same (without expecting
  /// implicit conversions). They can also both return nothing `void`.
  ///
  ///
  /// # Examples
  ///
  /// ```
  /// auto i = make_ok<int, string_view>(99);
  ///
  /// auto j = move(i).match([](int value) { return value; },
  ///                        [](string_view) { return -1; });
  /// ASSERT_EQ(j, 99);
  ///
  ///
  /// auto x = make_err<int, string_view>("404 Not Found"sv);
  /// // you can return nothing
  /// move(x).match([](int) {  },
  ///               [](string_view s) { fmt::print("Error: {}\n", s); });
  /// ```
  template <OneArgInvocable<T> OkFn, OneArgInvocable<E> ErrFn>
  requires same_as<invoke_result<OkFn const&, T>,
                   invoke_result<ErrFn const&, E>>  //
      auto match(OkFn const& ok_fn, ErrFn const& err_fn) && {
    if (is_ok()) {
      return ok_fn(std::move(value_ref_()));
    } else {
      return err_fn(std::move(err_ref_()));
    }
  }

  /// Calls the parameter `ok_fn` with the value if this result is an `Ok<T>`,
  /// else calls `err_fn` with the error. This result is consumed afterward.
  ///
  /// The return type of both parameters must be same (without expecting
  /// implicit conversions). They can also both return nothing `void`.
  ///
  ///
  /// # Examples
  ///
  /// ```
  /// auto i = make_ok<int, string_view>(99);
  ///
  /// auto j = move(i).match([](int value) { return value; },
  ///                        [](string_view) { return -1; });
  /// ASSERT_EQ(j, 99);
  ///
  ///
  /// auto x = make_err<int, string_view>("404 Not Found"sv);
  /// // you can return nothing
  /// move(x).match([](int) {  },
  ///               [](string_view s) { fmt::print("Error: {}\n", s); });
  /// ```
  template <OneArgInvocable<T> OkFn, OneArgInvocable<E> ErrFn>
  requires same_as<invoke_result<OkFn const&, T>, invoke_result<ErrFn&, E>>  //
      auto match(OkFn const& ok_fn, ErrFn& err_fn) && {  // NOLINT
    if (is_ok()) {
      return ok_fn(std::move(value_ref_()));
    } else {
      return err_fn(std::move(err_ref_()));
    }
  }

  /// Calls the parameter `ok_fn` with the value if this result is an `Ok<T>`,
  /// else calls `err_fn` with the error. This result is consumed afterward.
  ///
  /// The return type of both parameters must be same (without expecting
  /// implicit conversions). They can also both return nothing `void`.
  ///
  ///
  /// # Examples
  ///
  /// ```
  /// auto i = make_ok<int, string_view>(99);
  ///
  /// auto j = move(i).match([](int value) { return value; },
  ///                        [](string_view) { return -1; });
  /// ASSERT_EQ(j, 99);
  ///
  ///
  /// auto x = make_err<int, string_view>("404 Not Found"sv);
  /// // you can return nothing
  /// move(x).match([](int) {  },
  ///               [](string_view s) { fmt::print("Error: {}\n", s); });
  /// ```
  template <OneArgInvocable<T> OkFn, OneArgInvocable<E> ErrFn>
  requires same_as<invoke_result<OkFn&, T>, invoke_result<ErrFn const&, E>>  //
      auto match(OkFn& ok_fn, ErrFn const& err_fn) && {  // NOLINT
    if (is_ok()) {
      return ok_fn(std::move(value_ref_()));
    } else {
      return err_fn(std::move(err_ref_()));
    }
  }

  /// Calls the parameter `ok_fn` with the value if this result is an `Ok<T>`,
  /// else calls `err_fn` with the error. This result is consumed afterward.
  ///
  /// The return type of both parameters must be same (without expecting
  /// implicit conversions). They can also both return nothing `void`.
  ///
  ///
  /// # Examples
  ///
  /// ```
  /// auto i = make_ok<int, string_view>(99);
  ///
  /// auto j = move(i).match([](int value) { return value; },
  ///                        [](string_view) { return -1; });
  /// ASSERT_EQ(j, 99);
  ///
  ///
  /// auto x = make_err<int, string_view>("404 Not Found"sv);
  /// // you can return nothing
  /// move(x).match([](int) {  },
  ///               [](string_view s) { fmt::print("Error: {}\n", s); });
  /// ```
  template <OneArgInvocable<T> OkFn, OneArgInvocable<E> ErrFn>
  requires same_as<invoke_result<OkFn&, T>, invoke_result<ErrFn&, E>>  //
      auto match(OkFn& ok_fn, ErrFn& err_fn) && {                      // NOLINT
    if (is_ok()) {
      return ok_fn(std::move(value_ref_()));
    } else {
      return err_fn(std::move(err_ref_()));
    }
  }

  auto clone() const
      -> Result<T, E> requires CopyConstructible<T>&& CopyConstructible<E> {
    if (is_ok()) {
      return Ok<T>(T{value_cref_()});
    } else {
      return Err<E>(E{err_cref_()});
    }
  }

 private:
  bool is_ok_;
  storage_type storage_;

  Ok<T>& value_wrap_ref_() {
    auto buffer = reinterpret_cast<Ok<T>*>(storage_.first());
    return *std::launder(buffer);
  }

  Ok<T> const& value_wrap_cref_() const {
    auto buffer = reinterpret_cast<Ok<T> const*>(storage_.first());
    return *std::launder(buffer);
  }

  T& value_ref_() { return value_wrap_ref_().value_; }

  T const& value_cref_() const { return value_wrap_cref_().value_; }

  Err<E>& err_wrap_ref_() {
    auto buffer = reinterpret_cast<Err<E>*>(storage_.second());
    return *std::launder(buffer);
  }

  Err<E> const& err_wrap_cref_() const {
    auto buffer = reinterpret_cast<Err<E> const*>(storage_.second());
    return *std::launder(buffer);
  }

  E& err_ref_() { return err_wrap_ref_().value_; }

  E const& err_cref_() const { return err_wrap_cref_().value_; }

  void construct_value_(Ok<T>&& value) {
    // is the destructor ever called?
    //
    // object is created here with a placement
    // we always call std::move so the object is moved and destructors can be
    // called by the moved object and thus leave this buffer representation in
    // an unspecified state
    auto buff_ptr = &storage_.first_ref();
    new (buff_ptr) Ok<T>(std::forward<Ok<T>>(value));
  }

  void construct_err_(Err<E>&& error) {
    auto buff_ptr = &storage_.second_ref();
    new (buff_ptr) Err<E>{std::forward<Err<E>>(error)};
  }
};

/// Helper function to construct an `Option<T>` with a `Some<T>` value.
/// if the template parameter is not specified, it is auto-deduced from the
/// parameter's value.
///
/// # Examples
///
/// ```
/// // these are some of the various ways to construct on Option<T> with a
/// // Some<T> value
/// Option g = Some(9);
/// Option h = Some<int>(9);
/// Option<int> i = Some(9);
/// auto j = make_some(9);
/// auto k = Option<int>(Some<int>(9));
/// auto l = Option<int>(Some(9));
/// // ... and a few more
///
/// // to make it easier and less verbose:
/// auto m = make_some(9);
/// ASSERT_EQ(m, Some(9));
///
/// auto n = make_some<int>(9);
/// ASSERT_EQ(m, Some(9));
///
/// // observe that m is constructed as an Option<int> (=Option<T>) and T (=int)
/// // is auto-deduced from make_some's parameter type.
/// ```
template <Swappable T>
inline auto make_some(T&& value) -> Option<T> {
  return Option<T>(Some<T>(std::forward<T>(value)));
}

/// Helper function to construct an `Option<T>` with a `None` value.
/// note that the value parameter `T` must be specified.
///
/// # Examples
///
/// ```
/// // these are some of the various ways to construct on Option<T> with
/// // a None value
/// Option<int> h = None;
/// auto i = make_none<int>();
/// Option j = make_none<int>();
/// Option<int> k = make_none<int>();
///
/// // to make it easier and less verbose:
/// auto m = make_none<int>();
/// ASSERT_EQ(m, None);
///
/// // observe that m is constructed as an Option<int> (=Option<T>) and T(=int).
/// ```
template <Swappable T>
inline auto make_none() -> Option<T> {
  return Option<T>(None);
}

/// Helper function to construct a `Result<T, E>` with an `Ok<T>` value.
/// if the template parameter `T` is not specified, it is auto-deduced from the
/// parameter's value.
/// NOTE: The error type `E` must be specified and is the first template
/// parameter.
///
/// # Examples
///
/// ```
/// // these are some of the various ways to construct on Result<T, E> with an
/// // Ok<T> value
/// Result<int, string> a = Ok(8);
/// Result<int, string> b = Ok<int>(8);
///
/// // to make it easier and less verbose:
/// auto c = make_ok<string, int>(9);
/// ASSERT_EQ(c, Ok(9));
///
/// auto d = make_ok<string, int>(9);
/// ASSERT_EQ(d, Ok(9));
///
/// // observe that c is constructed as Result<int, string>
/// // (=Result<T, E>).
/// ```
template <Swappable T, Swappable E>
inline auto make_ok(T&& value) -> Result<T, E> {
  return Result<T, E>(Ok<T>(std::forward<T>(value)));
}

/// Helper function to construct a `Result<T, E>` with an `Err<E>` value.
/// if the template parameter `E` is not specified, it is auto-deduced from the
/// parameter's value.
/// NOTE: The value type `T` must be specified and is the first template
/// parameter.
///
/// # Examples
///
/// ```
/// // these are some of the various ways to construct on Result<T, E> with an
/// // Ok<T> value
/// Result<int, string> a = Err("foo"s);
/// Result<int, string> b = Err<string>("foo"s);
///
/// // to make it easier and less verbose:
/// auto c = make_err<int, string>("bar"s);
/// ASSERT_EQ(c, Err("bar"s));
///
/// // observe that c is constructed as Result<int, string>
/// // (=Result<T, E>).
/// ```
template <Swappable T, Swappable E>
inline auto make_err(E&& err) -> Result<T, E> {
  return Result<T, E>(Err<E>(std::forward<E>(err)));
}

};  // namespace stx

#endif  // STX_OPTION_RESULT_H_
