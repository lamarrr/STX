/**
 * @file option.h
 * @author Basit Ayantunde <rlamarrr@gmail.com>
 * @date 2020-04-11
 *
 * @copyright MIT License
 *
 * Copyright (c) 2020-2022 Basit Ayantunde
 *
 */

#pragma once

#include <string_view>
#include <utility>

#include "stx/enable_if.h"
#include "stx/none.h"
#include "stx/option_result/impl/check_value_type.h"
#include "stx/option_result/impl/option_storage.h"
#include "stx/option_result/impl/panic_helpers.h"
#include "stx/some.h"

STX_BEGIN_NAMESPACE

/// Optional values.
///
/// Type `Option` represents an optional value: every `Option`
/// is either `Some` and contains a value, or `None`, and
/// does not.
/// They have a number of uses:
///
/// * Initial values
/// * Return values for functions that are not defined over their entire input
/// range (partial functions)
/// * Return value for otherwise reporting simple errors, where `None` is
/// returned on error
/// * Optional struct fields
/// * Struct fields that can be loaned or "taken"
/// * Optional function arguments
/// * Nullable pointers
/// * Swapping things out of difficult situations
///
/// `Option`'s are commonly paired with pattern matching to query the
/// presence of a value and take action, always accounting for the `None`s
/// case.
///
/// ```
/// auto divide = [](double numerator, double denominator) -> Option<double> {
///   if (denominator == 0.0) {
///     return None;
///   } else {
///     return Some(numerator / denominator);
///   }
/// };
///
/// // The return value of the function is an option
/// auto result = divide(2.0, 3.0);
/// result.match([](double& value) { std::cout << value << std::endl; },
///              []() { std::cout << "has no value" << std::endl; });
/// ```
///
///
/// # Constexpr ?
///
/// C++ 20 and above
///
template <typename T>
struct [[nodiscard]] Option : impl::check_value_type<T>, private OptionStorage<T, std::is_trivial_v<T>>
{
public:
  using value_type = T;
  using storage    = OptionStorage<T, std::is_trivial_v<T>>;

private:
  using storage::some_;

public:
  constexpr Option() :
      storage{None}
  {}

  constexpr Option(Some<T> &&some) :
      storage{std::move(some)}
  {}

  constexpr Option(NoneType) :
      storage{None}
  {}

  constexpr Option &operator=(Some<T> &&other)
  {
    storage::assign(std::move(other));
    return *this;
  }

  constexpr Option &operator=(NoneType)
  {
    storage::assign(None);
    return *this;
  }

  constexpr Option(Option &&)            = default;
  constexpr Option &operator=(Option &&) = default;

  constexpr Option(Option const &)            = default;
  constexpr Option &operator=(Option const &) = default;

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
  [[nodiscard]] constexpr bool is_some() const
  {
    return !is_none();
  }

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
  [[nodiscard]] constexpr bool is_none() const
  {
    return storage::is_none_;
  }

  [[nodiscard]] constexpr operator bool() const
  {
    return is_some();
  }

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
  [[nodiscard]] constexpr bool contains(CmpType const &cmp) const
  {
    static_assert(equality_comparable<T, CmpType>);
    if (is_some())
    {
      return some_.cref() == cmp;
    }
    else
    {
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
  [[nodiscard]] constexpr bool exists(UnaryPredicate &&predicate) const
  {
    static_assert(invocable<UnaryPredicate &&, T const &>);
    static_assert(convertible<invoke_result<UnaryPredicate &&, T const &>, bool>);

    if (is_some())
    {
      return std::invoke(std::forward<UnaryPredicate>(predicate), some_.cref());
    }
    else
    {
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
  T &value() &
  {
    if (is_none())
    {
      impl::option::no_lref();
    }
    else
    {
      return some_.ref();
    }
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
  T const &value() const &
  {
    if (is_none())
    {
      impl::option::no_lref();
    }
    else
    {
      return some_.cref();
    }
  }

  T &&value() &&
  {
    if (is_none())
    {
      impl::option::no_lref();
    }
    else
    {
      return some_.move();
    }
  }

  /// Converts from `Option<T> const&` or `Option<T> &` to
  /// `Option<Ref<T const>>`.
  ///
  /// # NOTE
  /// `Ref<T const>` is an alias for `std::reference_wrapper<T const>` and
  /// guides against reference-collapsing
  constexpr auto as_cref() const & -> Option<Ref<T const>>
  {
    if (is_some())
    {
      return Some(Ref<T const>{some_.cref()});
    }
    else
    {
      return None;
    }
  }

  [[deprecated("calling Option::as_cref() on an r-value, and therefore binding a reference to an object that is marked to be moved")]]        //
  constexpr auto
      as_cref() const && -> Option<Ref<T const>> = delete;

  /// Converts from `Option<T>` to `Option<Ref<T>>`.
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
  constexpr auto as_ref() & -> Option<Ref<T>>
  {
    if (is_some())
    {
      return Some(Ref<T>(some_.ref()));
    }
    else
    {
      return None;
    }
  }

  constexpr auto as_ref() const & -> Option<Ref<T const>>
  {
    return as_cref();
  }

  [[deprecated("calling Option::as_ref() on an r-value, and therefore binding a reference to an object that is marked to be moved")]]        //
  constexpr auto
      as_ref() && -> Option<Ref<T>> = delete;

  [[deprecated("calling Option::as_ref() on an r-value, and therefore binding a reference to an object that is marked to be moved")]]        //
  constexpr auto
      as_ref() const && -> Option<Ref<T const>> = delete;

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
  auto expect(std::string_view msg) && -> T
  {
    if (is_some())
    {
      return some_.move();
    }
    else
    {
      impl::option::expect_value_failed(msg);
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
  auto unwrap() && -> T
  {
    if (is_some())
    {
      return some_.move();
    }
    else
    {
      impl::option::no_value();
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
  constexpr auto unwrap_or(T &&alt) && -> T
  {
    if (is_some())
    {
      return some_.move();
    }
    else
    {
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
  constexpr auto unwrap_or_else(Fn &&op) && -> T
  {
    static_assert(invocable<Fn &&>);
    if (is_some())
    {
      return some_.move();
    }
    else
    {
      return std::invoke(std::forward<Fn>(op));
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
  constexpr auto map(Fn &&op) && -> Option<invoke_result<Fn &&, T &&>>
  {
    static_assert(invocable<Fn &&, T &&>);
    if (is_some())
    {
      return Some(std::invoke(std::forward<Fn>(op), some_.move()));
    }
    else
    {
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
  template <typename Fn, typename Alt>
  constexpr auto map_or(Fn &&op, Alt &&alt) && -> invoke_result<Fn &&, T &&>
  {
    static_assert(invocable<Fn &&, T &&>);
    if (is_some())
    {
      return std::invoke(std::forward<Fn>(op), some_.move());
    }
    else
    {
      return std::forward<Alt>(alt);
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
  constexpr auto map_or_else(Fn &&op, AltFn &&alt_fn) && -> invoke_result<Fn &&, T &&>
  {
    static_assert(invocable<Fn &&, T &&>);
    static_assert(invocable<AltFn &&>);

    if (is_some())
    {
      return std::invoke(std::forward<Fn>(op), some_.move());
    }
    else
    {
      return std::invoke(std::forward<AltFn>(alt_fn));
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
  template <typename U>        //
  constexpr auto AND(Option<U> &&cmp) && -> Option<U>
  {
    if (is_some())
    {
      return std::move(cmp);
    }
    else
    {
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
  constexpr auto and_then(Fn &&op) && -> invoke_result<Fn &&, T &&>
  {
    static_assert(invocable<Fn &&, T &&>);
    if (is_some())
    {
      return std::invoke(std::forward<Fn>(op), some_.move());
    }
    else
    {
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
  constexpr auto filter(UnaryPredicate &&predicate) && -> Option
  {
    static_assert(invocable<UnaryPredicate &&, T const &>);
    static_assert(convertible<invoke_result<UnaryPredicate &&, T const &>, bool>);

    if (is_some() && std::invoke(std::forward<UnaryPredicate>(predicate), some_.cref()))
    {
      return std::move(*this);
    }
    else
    {
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
  constexpr auto OR(Option &&alt) && -> Option
  {
    if (is_some())
    {
      return std::move(*this);
    }
    else
    {
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
  constexpr auto or_else(Fn &&op) && -> Option
  {
    static_assert(invocable<Fn &&>);
    if (is_some())
    {
      return std::move(*this);
    }
    else
    {
      return std::invoke(std::forward<Fn>(op));
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
  constexpr auto take() -> Option
  {
    if (is_some())
    {
      Some some = std::move(some_);
      storage::assign(None);
      return some;
    }
    else
    {
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
  constexpr auto replace(T &&replacement) -> Option
  {
    if (is_some())
    {
      Some old = std::move(some_);
      storage::assign(Some(std::move(replacement)));
      return std::move(old);
    }
    else
    {
      storage::assign(Some(std::move(replacement)));
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
  auto replace(T const &replacement) -> Option
  {
    return replace(T{replacement});
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
  void expect_none(std::string_view msg)
  {
    if (is_some())
    {
      impl::option::expect_none_failed(msg);
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
  void unwrap_none()
  {
    if (is_some())
    {
      impl::option::no_none();
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
  constexpr auto unwrap_or_default() && -> T
  {
    static_assert(default_constructible<T>);
    if (is_some())
    {
      return some_.move();
    }
    else
    {
      return T{};
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
  constexpr auto match(SomeFn &&some_fn, NoneFn &&none_fn) && -> invoke_result<SomeFn &&, T &&>
  {
    static_assert(invocable<SomeFn &&, T &&>);
    static_assert(invocable<NoneFn &&>);

    if (is_some())
    {
      return std::invoke(std::forward<SomeFn>(some_fn), some_.move());
    }
    else
    {
      return std::invoke(std::forward<NoneFn>(none_fn));
    }
  }

  template <typename SomeFn, typename NoneFn>
  constexpr auto match(SomeFn &&some_fn, NoneFn &&none_fn) & -> invoke_result<SomeFn &&, T &>
  {
    static_assert(invocable<SomeFn &&, T &>);
    static_assert(invocable<NoneFn &&>);

    if (is_some())
    {
      return std::invoke(std::forward<SomeFn>(some_fn), some_.ref());
    }
    else
    {
      return std::invoke(std::forward<NoneFn>(none_fn));
    }
  }

  template <typename SomeFn, typename NoneFn>
  constexpr auto match(SomeFn &&some_fn, NoneFn &&none_fn) const & -> invoke_result<SomeFn &&, T const &>
  {
    static_assert(invocable<SomeFn &&, T const &>);
    static_assert(invocable<NoneFn &&>);

    if (is_some())
    {
      return std::invoke(std::forward<SomeFn>(some_fn), some_.cref());
    }
    else
    {
      return std::invoke(std::forward<NoneFn>(none_fn));
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
  /// ASSERT_EQ(x, x.copy());
  /// ```
  constexpr auto copy() const -> Option
  {
    return *this;
  }

  constexpr auto move() & -> Option<T> &&
  {
    return std::move(*this);
  }

  constexpr auto move() && -> Option<T> && = delete;

  constexpr Some<T> &unsafe_some_ref()
  {
    return some_;
  }

  constexpr Some<T> const &unsafe_some_ref() const
  {
    return some_;
  }
};

template <typename T, typename U, STX_ENABLE_IF(equality_comparable<T, U>)>
[[nodiscard]] constexpr bool operator==(Option<T> const &a, Option<U> const &b)
{
  if (a.is_some() && b.is_some())
  {
    return a.unsafe_some_ref() == b.unsafe_some_ref();
  }
  else if (a.is_none() && b.is_none())
  {
    return true;
  }
  else
  {
    return false;
  }
}

template <typename T, typename U, STX_ENABLE_IF(equality_comparable<T, U>)>
[[nodiscard]] constexpr bool operator!=(Option<T> const &a, Option<U> const &b)
{
  if (a.is_some() && b.is_some())
  {
    return a.unsafe_some_ref() != b.unsafe_some_ref();
  }
  else if (a.is_none() && b.is_none())
  {
    return false;
  }
  else
  {
    return true;
  }
}

template <typename T, typename U, STX_ENABLE_IF(equality_comparable<T, U>)>
[[nodiscard]] constexpr bool operator==(Option<T> const &a, Some<U> const &b)
{
  if (a.is_some())
  {
    return a.unsafe_some_ref() == b;
  }
  else
  {
    return false;
  }
}

template <typename T, typename U, STX_ENABLE_IF(equality_comparable<T, U>)>
[[nodiscard]] constexpr bool operator!=(Option<T> const &a, Some<U> const &b)
{
  if (a.is_some())
  {
    return a.unsafe_some_ref() != b;
  }
  else
  {
    return true;
  }
}

template <typename U, typename T, STX_ENABLE_IF(equality_comparable<T, U>)>
[[nodiscard]] constexpr bool operator==(Some<U> const &a, Option<T> const &b)
{
  if (b.is_some())
  {
    return a == b.unsafe_some_ref();
  }
  else
  {
    return false;
  }
}

template <typename U, typename T, STX_ENABLE_IF(equality_comparable<T, U>)>
[[nodiscard]] constexpr bool operator!=(Some<U> const &a, Option<T> const &b)
{
  if (b.is_some())
  {
    return a != b.unsafe_some_ref();
  }
  else
  {
    return true;
  }
}

template <typename T>
[[nodiscard]] constexpr bool operator==(Option<T> const &a, NoneType const &)
{
  return a.is_none();
}

template <typename T>
[[nodiscard]] constexpr bool operator!=(Option<T> const &a, NoneType const &)
{
  return a.is_some();
}

template <typename T>
[[nodiscard]] constexpr bool operator==(NoneType const &, Option<T> const &a)
{
  return a.is_none();
}

template <typename T>
[[nodiscard]] constexpr bool operator!=(NoneType const &, Option<T> const &a)
{
  return a.is_some();
}

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
constexpr auto make_some(T &&value) -> Option<T>
{
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
constexpr auto make_none() -> Option<T>
{
  return None;
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
constexpr auto some_ref(T &value)
{
  return Some(Ref<T>{value});
}

STX_END_NAMESPACE
