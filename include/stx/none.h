
#pragma once

#include "stx/config.h"

STX_BEGIN_NAMESPACE

/// value-variant Type for `Option<T>` representing no-value
///
/// # Constexpr ?
///
/// C++ 17 and above
///
struct [[nodiscard]] NoneType
{};

/// value-variant for `Option<T>` representing no-value
constexpr NoneType const None{};

[[nodiscard]] constexpr bool operator==(NoneType const &, NoneType const &)
{
  return true;
}

[[nodiscard]] constexpr bool operator!=(NoneType const &, NoneType const &)
{
  return false;
}

STX_END_NAMESPACE
