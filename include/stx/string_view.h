

#pragma once
#include <cstddef>
#include <cstring>

#include "stx/config.h"
#include "stx/option.h"
#include "stx/span.h"

STX_BEGIN_NAMESPACE


// TODO(lamarrr): replace all usages of std::string_view with this
// guaranteed to be null-terminated
struct StringView
{
  using Iterator = char const *;
  using Pointer  = char const *;
  using Size     = size_t;
  using Index    = size_t;

  static constexpr Size length(char const *c_str)
  {
    char const *it = c_str;
    while (*it != 0)
    {
      it++;
    }
    return it - c_str;
  }

  constexpr StringView(char const *c_string) :
      data_{c_string}, size_{length(c_string)}
  {
  }

  constexpr StringView(char const *c_string, Size size) :
      data_{c_string}, size_{size}
  {
  }

  // TODO(lamarrr): add constructor that takes c_str() and or size()

  constexpr char const *c_str() const
  {
    return data_;
  }

  constexpr Pointer data() const
  {
    return data_;
  }

  constexpr Size size() const
  {
    return size_;
  }

  constexpr Iterator begin() const
  {
    return data_;
  }

  constexpr Iterator end() const
  {
    return data_ + size_;
  }

  constexpr bool is_empty() const
  {
    return size_ == 0;
  }

  char const &operator[](Index index) const
  {
    return span()[index];
  }

  Option<Ref<char const>> at(Index index) const
  {
    return span().at(index);
  }

  constexpr bool starts_with(StringView other) const
  {
    if (other.size_ > size_)
    {
      return false;
    }

    return std::memcmp(data_, other.data_, other.size_) == 0;
  }

  constexpr bool starts_with(char c) const
  {
    return size_ > 0 && data_[0] == c;
  }

  constexpr bool ends_with(StringView other) const
  {
    if (other.size_ > size_)
    {
      return false;
    }

    return std::memcmp(data_ + (size_ - other.size_), other.data_, other.size_) == 0;
  }

  // TODO(lamarrr) ::contains(), add to String as well

  constexpr bool ends_with(char c) const
  {
    return size_ > 0 && data_[size_ - 1] == c;
  }

  constexpr stx::Span<char const> span() const
  {
    return stx::Span<char const>{data_, size_};
  }

  constexpr bool operator==(StringView other) const
  {
    if (size_ != other.size_)
    {
      return false;
    }

    return std::memcmp(data_, other.data_, size_) == 0;
  }

  constexpr bool operator!=(StringView other) const
  {
    return !(*this == other);
  }

  char const *data_ = "";
  Size        size_ = 0;
};

STX_END_NAMESPACE
