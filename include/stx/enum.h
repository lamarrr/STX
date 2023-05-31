#pragma once
#include <type_traits>

#include "stx/config.h"

STX_BEGIN_NAMESPACE

template <typename EnumType>
using enum_ut = std::underlying_type_t<EnumType>;

template <typename EnumType>
constexpr enum_ut<EnumType> enum_uv(EnumType a)
{
  return static_cast<enum_ut<EnumType>>(a);
}

template <typename EnumType>
constexpr enum_ut<EnumType> enum_uv_or(EnumType a, EnumType b)
{
  return static_cast<enum_ut<EnumType>>(enum_uv(a) | enum_uv(b));
}

template <typename EnumType>
constexpr EnumType enum_or(EnumType a, EnumType b)
{
  return static_cast<EnumType>(enum_uv_or(a, b));
}

template <typename EnumType>
constexpr enum_ut<EnumType> enum_uv_and(EnumType a, EnumType b)
{
  return static_cast<enum_ut<EnumType>>(enum_uv(a) & enum_uv(b));
}

template <typename EnumType>
constexpr enum_ut<EnumType> enum_uv_toggle(EnumType a)
{
  return static_cast<enum_ut<EnumType>>(~enum_uv(a));
}

template <typename EnumType>
constexpr EnumType enum_toggle(EnumType a)
{
  return static_cast<EnumType>(enum_uv_toggle(a));
}

template <typename EnumType>
constexpr EnumType enum_and(EnumType a, EnumType b)
{
  return static_cast<EnumType>(enum_uv_and(a, b));
}

#define STX_DEFINE_ENUM_BIT_OPS(enum_type)                   \
  constexpr enum_type operator|(enum_type a, enum_type b)    \
  {                                                          \
    return ::stx::enum_or(a, b);                             \
  }                                                          \
                                                             \
  constexpr enum_type operator~(enum_type a)                 \
  {                                                          \
    return ::stx::enum_toggle(a);                            \
  }                                                          \
                                                             \
  constexpr enum_type &operator|=(enum_type &a, enum_type b) \
  {                                                          \
    a = a | b;                                               \
    return a;                                                \
  }                                                          \
                                                             \
  constexpr enum_type operator&(enum_type a, enum_type b)    \
  {                                                          \
    return ::stx::enum_and(a, b);                            \
  }                                                          \
                                                             \
  constexpr enum_type &operator&=(enum_type &a, enum_type b) \
  {                                                          \
    a = a & b;                                               \
    return a;                                                \
  }

STX_END_NAMESPACE
