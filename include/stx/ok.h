#pragma once

#include <utility>

#include "stx/config.h"
#include "stx/option_result/impl/check_value_type.h"
#include "stx/utils/common.h"
#include "stx/utils/enable_if.h"

STX_BEGIN_NAMESPACE

/// value-variant for `Result<T, E>` wrapping the contained successful value of
/// type `T`
///
/// Note that `Ok` is only a value-forwarding type.
/// An `Ok<T>` can only be constructed with an r-value. What does this mean?
///
/// For example, You can:
///
/// ```cpp
/// Result<vector<int>, int> a = Ok(vector{1, 2, 3, 4});
/// ```
/// You can't:
///
/// ```cpp
/// vector<int> x {1, 2, 3, 4};
/// Result<vector<int>, int> a = Ok(x);
/// ```
/// But, to explicitly make `a` take ownership, you will:
///
/// ```cpp
/// vector<int> x {1, 2, 3, 4};
/// Result<vector<int>, int> a = Ok(std::move(x));
/// ```
///
/// # Constexpr ?
///
/// C++ 17 and above
///
template <typename T>
struct [[nodiscard]] Ok : impl::check_value_type<T> {
  using value_type = T;

  template <typename Tp, typename Er>
  friend struct Result;

  /// an `Ok<T>` can only be constructed with an r-value of type `T`
  explicit constexpr Ok(T && value) : value_(std::move(value)) {}

  constexpr T copy() const { return value_; }

  constexpr T&& move() { return std::move(value_); }

  constexpr T& ref() { return value_; }

  constexpr T const& cref() const { return value_; }

 private:
  T value_;
};

template <typename T, typename U, STX_ENABLE_IF(equality_comparable<T, U>)>
[[nodiscard]] constexpr bool operator==(Ok<T> const& a, Ok<U> const& b) {
  return a.cref() == b.cref();
}

template <typename T, typename U, STX_ENABLE_IF(equality_comparable<T, U>)>
[[nodiscard]] constexpr bool operator!=(Ok<T> const& a, Ok<U> const& b) {
  return a.cref() != b.cref();
}

STX_END_NAMESPACE
