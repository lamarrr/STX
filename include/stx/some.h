
#pragma once

#include <utility>

#include "stx/common.h"
#include "stx/config.h"
#include "stx/enable_if.h"

STX_BEGIN_NAMESPACE

/// value-variant for `Option<T>` wrapping the contained value
///
/// # Usage
///
/// Note that `Some` is only a value-forwarding type. It doesn't make copies of
/// its constructor arguments and only accepts r-values.
///
/// What does this mean?
///
/// For example, You can:
///
/// ```cpp
/// Option a = Some(vector{1, 2, 3, 4});
/// ```
/// You can't:
///
/// ```cpp
/// vector<int> x {1, 2, 3, 4};
/// Option a = Some(x);
/// ```
/// But, to explicitly make `a` take ownership, you will:
///
/// ```cpp
/// vector<int> x {1, 2, 3, 4};
/// Option a = Some(std::move(x));
/// ```
///
/// # Constexpr ?
///
/// C++ 17 and above
///
template <typename T>
struct [[nodiscard]] Some : impl::check_value_type<T>
{
  using value_type = T;

  template <typename Tp>
  friend struct Option;

  /// a `Some<T>` can only be constructed with an r-value of type `T`
  explicit constexpr Some(T &&value) :
      value_{std::move(value)}
  {}

  constexpr T copy() const
  {
    return value_;
  }

  constexpr T &&move()
  {
    return std::move(value_);
  }

  constexpr T &ref()
  {
    return value_;
  }

  constexpr T const &cref() const
  {
    return value_;
  }

private:
  T value_;
};

template <typename T, typename U, STX_ENABLE_IF(equality_comparable<T, U>)>
[[nodiscard]] constexpr bool operator==(Some<T> const &a, Some<U> const &b)
{
  return a.cref() == b.cref();
}

template <typename T, typename U, STX_ENABLE_IF(equality_comparable<T, U>)>
[[nodiscard]] constexpr bool operator!=(Some<T> const &a, Some<U> const &b)
{
  return a.cref() != b.cref();
}

STX_END_NAMESPACE
