
#pragma once

#include <utility>

#include "stx/config.h"
#include "stx/option_result/impl/check_value_type.h"
#include "stx/utils/common.h"
#include "stx/utils/enable_if.h"

STX_BEGIN_NAMESPACE

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
struct [[nodiscard]] Err : impl::check_value_type<E> {
  using value_type = E;

  template <typename Tp, typename Er>
  friend struct Result;

  /// an `Err<E>` can only be constructed with an r-value of type `E`
  explicit constexpr Err(E && value) : value_(std::move(value)) {}

  constexpr E copy() const { return value_; }

  constexpr E&& move() { return std::move(value_); }

  constexpr E& ref() { return value_; }

  constexpr E const& cref() const { return value_; }

 private:
  E value_;
};

template <typename E, typename F, STX_ENABLE_IF(equality_comparable<E, F>)>
[[nodiscard]] constexpr bool operator==(Err<E> const& a, Err<F> const& b) {
  return a.cref() == b.cref();
}

template <typename E, typename F, STX_ENABLE_IF(equality_comparable<E, F>)>
[[nodiscard]] constexpr bool operator!=(Err<E> const& a, Err<F> const& b) {
  return a.cref() != b.cref();
}

STX_END_NAMESPACE
