/**
 * @file option_result.h
 * @author Basit Ayantunde <rlamarrr@gmail.com>
 * @date 2020-04-16
 *
 * @copyright MIT License
 *
 * Copyright (c) 2020-2022 Basit Ayantunde
 *
 */

#pragma once

#include <string_view>
#include <utility>

#include "stx/common.h"
#include "stx/config.h"
#include "stx/option_result/impl/check_value_type.h"
#include "stx/option_result/impl/panic_helpers.h"
#include "stx/option_result/impl/result_storage.h"

// Notes:
// - Result is an object-forwarding type. It is unique to an
// interaction. It functions like a std::unique_ptr, as it doesn't allow
// implicitly copying its data content. Unless explicitly stated via the
// .copy() method
// - We strive to make lifetime paths as visible and predictable as
// possible
// - We also try to prevent you from shooting yourself in the foot, especially
// with references and implicit copies

/// @file
///
/// to run tests, use:
///
/// ``` cpp
///
/// #include <iostream>
/// #include <string>
/// #include <string_view>
///
///
/// using std::move, std::string, std::string_view;
/// using namespace std::literals; // makes '"Hello"s' give std::string
///                                // and '"Hello"sv' give std::string_view
///
/// ```

STX_BEGIN_NAMESPACE

/// ### Error handling with the `Result` type.
///
/// `Result<T, E>` is a type used for returning and propagating
/// errors. It is a class with the variants: `Ok<T>`, representing
/// success and containing a value, and `Err<E>`, representing error
/// and containing an error value.
///
///
/// Functions return `Result` whenever errors are expected and
/// recoverable.
///
/// A simple function returning `Result` might be
/// defined and used like so:
///
/// ``` cpp
/// enum class Version { Version1 = 1, Version2 = 2 };
///
/// auto parse_version =
///      [](array<uint8_t, 5> const& header) -> Result<Version, string_view> {
///    switch (header.at(0)) {
///      case 1:
///        return Ok(Version::Version1);
///      case 2:
///        return Ok(Version::Version2);
///      default:
///        return Err("invalid version"sv);
///    }
///  };
///
/// parse_version({1, 2, 3, 4, 5})
///      .match(
///          [](auto version) {
///            std::cout << "Working with version: "
///                      << static_cast<int>(version) << "\n";
///          },
///          [](auto err) {
///            std::cout << "Error parsing header: " << err << "\n";
///          });
/// ```
///
///
/// `Result` comes with some convenience methods that make working with it more
/// succinct.
///
/// ``` cpp
/// Result<int, int> good_result = Ok(10);
/// Result<int, int> bad_result = Err(10);
///
/// // The `is_ok` and `is_err` methods do what they say.
/// ASSERT_TRUE(good_result.is_ok() && !good_result.is_err());
/// ASSERT_TRUE(bad_result.is_err() && !bad_result.is_ok());
/// ```
///
/// `Result` is a type that represents either success (`Ok`) or failure (`Err`).
///
/// Result is either in the Ok or Err state at any point in time
///
/// # Constexpr ?
///
/// C++ 20 and above
///
/// # Note
///
/// `Result` unlike `Option` is a value-forwarding type. It doesn't have copy
/// constructors of any sort. More like a `unique_ptr`.
///
/// `Result` should be seen as a return channel (for returning from functions)
/// and not an object.
///

template <typename T, typename E>
struct [[nodiscard]] Result : impl::check_value_type<T>, impl::check_err_type<E>, private ResultStorage<T, E, std::is_trivial_v<T> && std::is_trivial_v<E>>
{
public:
  using value_type = T;
  using error_type = E;
  using storage    = ResultStorage<T, E, std::is_trivial_v<T> && std::is_trivial_v<E>>;

private:
  using storage::err_;
  using storage::ok_;

public:
  constexpr Result(Ok<T> &&ok) :
      storage{std::move(ok)}
  {}

  constexpr Result(Err<E> &&err) :
      storage{std::move(err)}
  {}

  constexpr Result &operator=(Ok<T> &&other)
  {
    storage::assign(std::move(other));

    return *this;
  }

  constexpr Result &operator=(Err<T> &&other)
  {
    storage::assign(std::move(other));

    return *this;
  }

  constexpr Result(Result &&)            = default;
  constexpr Result &operator=(Result &&) = default;

  constexpr Result()                               = delete;
  constexpr Result(Result const &other)            = delete;
  constexpr Result &operator=(Result const &other) = delete;

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
  [[nodiscard]] constexpr bool is_ok() const
  {
    return storage::is_ok_;
  }

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
  [[nodiscard]] constexpr bool is_err() const
  {
    return !is_ok();
  }

  [[nodiscard]] constexpr operator bool() const
  {
    return is_ok();
  }

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
  [[nodiscard]] constexpr bool contains(CmpType const &cmp) const
  {
    static_assert(equality_comparable<T, CmpType>);
    if (is_ok())
    {
      return ok_.cref() == cmp;
    }
    else
    {
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
  [[nodiscard]] constexpr bool contains_err(ErrCmp const &cmp) const
  {
    static_assert(equality_comparable<E, ErrCmp>);
    if (is_ok())
    {
      return false;
    }
    else
    {
      return err_.cref() == cmp;
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
  [[nodiscard]] constexpr bool exists(UnaryPredicate &&predicate) const
  {
    static_assert(invocable<UnaryPredicate &&, T const &>);
    static_assert(convertible<invoke_result<UnaryPredicate &&, T const &>, bool>);

    if (is_ok())
    {
      return std::invoke(std::forward<UnaryPredicate>(predicate), ok_.cref());
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
  /// Result<int, string> x = Err("invalid"s);
  /// auto invalid = [](auto x) { return x == "invalid"; };
  ///
  /// ASSERT_TRUE(x.err_exists(invalid));
  ///
  /// ```
  template <typename UnaryPredicate>
  [[nodiscard]] constexpr bool err_exists(UnaryPredicate &&predicate) const
  {
    static_assert(invocable<UnaryPredicate &&, E const &>);
    static_assert(convertible<invoke_result<UnaryPredicate &&, E const &>, bool>);

    if (is_err())
    {
      return std::invoke(std::forward<UnaryPredicate>(predicate), err_.cref());
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
  T &value() &
  {
    if (is_err())
    {
      impl::result::no_lref(err_.cref());
    }
    else
    {
      return ok_.ref();
    }
  }

  T const &value() const &
  {
    if (is_err())
    {
      impl::result::no_lref(err_.cref());
    }
    else
    {
      return ok_.cref();
    }
  }

  T &&value() &&
  {
    if (is_err())
    {
      impl::result::no_lref(err_.cref());
    }
    else
    {
      return ok_.move();
    }
  }

  /// Returns a reference to the contained error value.
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
  /// int& err = result.err();
  /// err = 46;
  ///
  /// ASSERT_EQ(result, Err(46));
  /// ```
  [[nodiscard]] E &err() &
  {
    if (is_ok())
    {
      impl::result::no_err_lref();
    }
    else
    {
      return err_.ref();
    }
  }

  [[nodiscard]] E const &err() const &
  {
    if (is_ok())
    {
      impl::result::no_err_lref();
    }
    else
    {
      return err_.cref();
    }
  }

  [[nodiscard]] E &&err() &&
  {
    if (is_ok())
    {
      impl::result::no_err_lref();
    }
    else
    {
      return err_.move();
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
  constexpr auto as_cref() const & -> Result<Ref<T const>, Ref<E const>>
  {
    if (is_ok())
    {
      return Ok(Ref<T const>{ok_.cref()});
    }
    else
    {
      return Err(Ref<E const>{err_.cref()});
    }
  }

  [[deprecated("calling Result::as_cref() on an r-value, and therefore binding an l-value reference to an object that is marked to be moved")]]        //
  constexpr auto
      as_cref() const && -> Result<Ref<T const>, Ref<E const>> = delete;

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
  constexpr auto as_ref() & -> Result<Ref<T>, Ref<E>>
  {
    if (is_ok())
    {
      return Ok(Ref<T>(ok_.ref()));
    }
    else
    {
      return Err(Ref<E>(err_.ref()));
    }
  }

  constexpr auto as_ref() const & -> Result<Ref<T const>, Ref<E const>>
  {
    return as_cref();
  }

  [[deprecated("calling Result::as_ref() on an r-value, and therefore binding a reference to an object that is marked to be moved")]]        //
  constexpr auto
      as_ref() && -> Result<Ref<T>, Ref<E>> = delete;

  [[deprecated("calling Result::as_ref() on an r-value, and therefore binding a reference to an object that is marked to be moved")]]        //
  constexpr auto
      as_ref() const && -> Result<Ref<T>, Ref<E>> = delete;

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
  constexpr auto map(Fn &&op) && -> Result<invoke_result<Fn &&, T &&>, E>
  {
    static_assert(invocable<Fn &&, T &&>);

    if (is_ok())
    {
      return Ok(std::invoke(std::forward<Fn>(op), ok_.move()));
    }
    else
    {
      return Err(err_.move());
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
  constexpr auto map_or(Fn &&op, AltType &&alt) && -> invoke_result<Fn &&, T &&>
  {
    static_assert(invocable<Fn &&, T &&>);

    if (is_ok())
    {
      return std::invoke(std::forward<Fn>(op), ok_.move());
    }
    else
    {
      return std::forward<AltType>(alt);
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
  constexpr auto map_or_else(Fn &&op,
                             A  &&alt_op) && -> invoke_result<Fn &&, T &&>
  {
    static_assert(invocable<Fn &&, T &&>);
    static_assert(invocable<A &&, E &&>);

    if (is_ok())
    {
      return std::invoke(std::forward<Fn>(op), ok_.move());
    }
    else
    {
      return std::forward<A>(alt_op)(err_.move());
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
  constexpr auto map_err(Fn &&op) && -> Result<T, invoke_result<Fn &&, E &&>>
  {
    static_assert(invocable<Fn &&, E &&>);

    if (is_ok())
    {
      return Ok(ok_.move());
    }
    else
    {
      return Err(std::invoke(std::forward<Fn>(op), err_.move()));
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
  constexpr auto AND(Result<U, F> &&res) && -> Result<U, F>
  {
    static_assert(convertible<E &&, F>);
    if (is_ok())
    {
      return std::forward<Result<U, F>>(res);
    }
    else
    {
      return Err(err_.move());
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
  constexpr auto and_then(Fn &&op) && -> Result<invoke_result<Fn &&, T &&>, E>
  {
    static_assert(invocable<Fn &&, T &&>);

    if (is_ok())
    {
      return Ok(std::invoke(std::forward<Fn>(op), ok_.move()));
    }
    else
    {
      return Err(err_.move());
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
  constexpr auto OR(Result &&alt) && -> Result
  {
    if (is_ok())
    {
      return Ok(ok_.move());
    }
    else
    {
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
  constexpr auto or_else(Fn &&op) && -> invoke_result<Fn &&, E &&>
  {
    static_assert(invocable<Fn &&, E &&>);

    if (is_ok())
    {
      return Ok(ok_.move());
    }
    else
    {
      return std::invoke(std::forward<Fn>(op), err_.move());
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
  constexpr auto unwrap_or(T &&alt) && -> T
  {
    if (is_ok())
    {
      return ok_.move();
    }
    else
    {
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
  constexpr auto unwrap_or_else(Fn &&op) && -> T
  {
    static_assert(invocable<Fn &&, E &&>);

    if (is_ok())
    {
      return ok_.move();
    }
    else
    {
      return std::invoke(std::forward<Fn>(op), err_.move());
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
  auto unwrap() && -> T
  {
    if (is_err())
    {
      impl::result::no_value(err_.cref());
    }
    else
    {
      return ok_.move();
    }
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
  auto expect(std::string_view msg) && -> T
  {
    if (is_err())
    {
      impl::result::expect_value_failed(msg, err_.cref());
    }
    else
    {
      return ok_.move();
    }
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
  [[nodiscard]] auto unwrap_err() && -> E
  {
    if (is_ok())
    {
      impl::result::no_err();
    }
    else
    {
      return err_.move();
    }
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
  [[nodiscard]] auto expect_err(std::string_view msg) && -> E
  {
    if (is_ok())
    {
      impl::result::expect_err_failed(msg);
    }
    else
    {
      return err_.move();
    }
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
  constexpr auto unwrap_or_default() && -> T
  {
    static_assert(default_constructible<T>);
    if (is_ok())
    {
      return ok_.move();
    }
    else
    {
      return T{};
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
  constexpr auto match(OkFn &&ok_fn, ErrFn &&err_fn) && -> invoke_result<OkFn &&, T &&>
  {
    static_assert(invocable<OkFn &&, T &&>);
    static_assert(invocable<ErrFn &&, E &&>);

    if (is_ok())
    {
      return std::invoke(std::forward<OkFn>(ok_fn), ok_.move());
    }
    else
    {
      return std::invoke(std::forward<ErrFn>(err_fn), err_.move());
    }
  }

  template <typename OkFn, typename ErrFn>
  constexpr auto match(OkFn &&ok_fn, ErrFn &&err_fn) & -> invoke_result<OkFn &&, T &>
  {
    static_assert(invocable<OkFn &&, T &>);
    static_assert(invocable<ErrFn &&, E &>);

    if (is_ok())
    {
      return std::invoke(std::forward<OkFn>(ok_fn), ok_.ref());
    }
    else
    {
      return std::invoke(std::forward<ErrFn>(err_fn), err_.ref());
    }
  }

  template <typename OkFn, typename ErrFn>
  constexpr auto match(OkFn &&ok_fn, ErrFn &&err_fn) const & -> invoke_result<OkFn &&, T const &>
  {
    static_assert(invocable<OkFn &&, T const &>);
    static_assert(invocable<ErrFn &&, E const &>);

    if (is_ok())
    {
      return std::invoke(std::forward<OkFn>(ok_fn), ok_.cref());
    }
    else
    {
      return std::invoke(std::forward<ErrFn>(err_fn), err_.cref());
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
  /// ASSERT_EQ(x, x.copy());
  /// ```
  constexpr auto copy() const -> Result<T, E>
  {
    static_assert(copy_constructible<T>);
    static_assert(copy_constructible<E>);

    if (is_ok())
    {
      return Ok(ok_.copy());
    }
    else
    {
      return Err(err_.copy());
    }
  }

  constexpr auto move() & -> Result<T, E> &&
  {
    return std::move(*this);
  }

  constexpr auto move() && -> Result<T, E> && = delete;

  constexpr Ok<T> &unsafe_ok_ref()
  {
    return ok_;
  }

  constexpr Ok<T> const &unsafe_ok_ref() const
  {
    return ok_;
  }

  constexpr Err<E> &unsafe_err_ref()
  {
    return err_;
  }

  constexpr Err<E> const &unsafe_err_ref() const
  {
    return err_;
  }
};

template <typename T, typename E, typename U, STX_ENABLE_IF(equality_comparable<T, U>)>
[[nodiscard]] constexpr bool operator==(Result<T, E> const &a, Ok<U> const &b)
{
  if (a.is_ok())
  {
    return a.unsafe_ok_ref() == b;
  }
  else
  {
    return false;
  }
}

template <typename T, typename E, typename U, STX_ENABLE_IF(equality_comparable<T, U>)>
[[nodiscard]] constexpr bool operator!=(Result<T, E> const &a, Ok<U> const &b)
{
  if (a.is_ok())
  {
    return a.unsafe_ok_ref() != b;
  }
  else
  {
    return true;
  }
}

template <typename U, typename T, typename E, STX_ENABLE_IF(equality_comparable<U, T>)>
[[nodiscard]] constexpr bool operator==(Ok<U> const &a, Result<T, E> const &b)
{
  if (b.is_ok())
  {
    return a == b.unsafe_ok_ref();
  }
  else
  {
    return false;
  }
}

template <typename U, typename T, typename E, STX_ENABLE_IF(equality_comparable<U, T>)>
[[nodiscard]] constexpr bool operator!=(Ok<U> const &a, Result<T, E> const &b)
{
  if (b.is_ok())
  {
    return a != b.unsafe_ok_ref();
  }
  else
  {
    return true;
  }
}

template <typename T, typename E, typename U, STX_ENABLE_IF(equality_comparable<E, U>)>
[[nodiscard]] constexpr bool operator==(Result<T, E> const &a, Err<U> const &b)
{
  if (a.is_err())
  {
    return a.unsafe_err_ref() == b;
  }
  else
  {
    return false;
  }
}

template <typename T, typename E, typename U, STX_ENABLE_IF(equality_comparable<E, U>)>
[[nodiscard]] constexpr bool operator!=(Result<T, E> const &a, Err<U> const &b)
{
  if (a.is_err())
  {
    return a.unsafe_err_ref() != b;
  }
  else
  {
    return true;
  }
}

template <typename U, typename T, typename E, STX_ENABLE_IF(equality_comparable<U, E>)>
[[nodiscard]] constexpr bool operator==(Err<U> const &a, Result<T, E> const &b)
{
  if (b.is_err())
  {
    return a == b.unsafe_err_ref();
  }
  else
  {
    return false;
  }
}

template <typename U, typename T, typename E, STX_ENABLE_IF(equality_comparable<U, E>)>
[[nodiscard]] constexpr bool operator!=(Err<U> const &a, Result<T, E> const &b)
{
  if (b.is_err())
  {
    return a != b.unsafe_err_ref();
  }
  else
  {
    return true;
  }
}

template <typename T, typename E, typename U, typename F, STX_ENABLE_IF(equality_comparable<T, U> &&equality_comparable<E, F>)>
[[nodiscard]] constexpr bool operator==(Result<T, E> const &a, Result<U, F> const &b)
{
  if (a.is_ok() && b.is_ok())
  {
    return a.unsafe_ok_ref() == b.unsafe_ok_ref();
  }
  else if (a.is_err() && b.is_err())
  {
    return a.unsafe_err_ref() == b.unsafe_err_ref();
  }
  else
  {
    return false;
  }
}

template <typename T, typename E, typename U, typename F, STX_ENABLE_IF(equality_comparable<T, U> &&equality_comparable<E, F>)>
[[nodiscard]] constexpr bool operator!=(Result<T, E> const &a, Result<U, F> const &b)
{
  if (a.is_ok() && b.is_ok())
  {
    return a.unsafe_ok_ref() != b.unsafe_ok_ref();
  }
  else if (a.is_err() && b.is_err())
  {
    return a.unsafe_err_ref() != b.unsafe_err_ref();
  }
  else
  {
    return true;
  }
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
constexpr auto make_ok(T &&value) -> Result<T, E>
{
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
constexpr auto make_err(E &&err) -> Result<T, E>
{
  return Err<E>(std::forward<E>(err));
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
constexpr auto ok_ref(T &value)
{
  return Ok(Ref<T>{value});
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
constexpr auto err_ref(E &value)
{
  return Err(Ref<E>{value});
}

STX_END_NAMESPACE
