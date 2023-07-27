
#pragma once

#include <cctype>
#include <cstddef>
#include <cstring>
#include <string_view>
#include <utility>

#include "stx/allocator.h"
#include "stx/c_string_view.h"
#include "stx/config.h"
#include "stx/memory.h"
#include "stx/option.h"
#include "stx/rc.h"
#include "stx/span.h"

STX_BEGIN_NAMESPACE

constexpr char const EMPTY_STRING[] = "";

//
// An owning read-only byte string.
//
// PROPERTIES:
// - No small string optimization (SSO)
// - always read-only
// - always null-terminated (for compatibility with C APIs, to prevent extra
// allocations for null-termination)
// - doesn't support copying from copy constructors
// - it's just a plain dumb sequence of characters/bytes
//
// with these attributes, we can avoid heap allocation of static strings.
// we can be move the strings across threads.
// the string can be accessed from multiple threads with no data race.
// the string is always valid as long as lifetime of `Str` is valid.
//
//
struct String
{
  using Iterator = char const *;
  using Pointer  = char const *;
  using Size     = size_t;
  using Index    = size_t;

  String() :
      memory_{static_storage_allocator, EMPTY_STRING}, size_{0}
  {}

  String(char const *static_storage_string_literal) :
      memory_{static_storage_allocator, static_storage_string_literal}, size_{CStringView::length(static_storage_string_literal)}
  {}

  String(ReadOnlyMemory memory, Size size) :
      memory_{std::move(memory)}, size_{size}
  {}

  String(String const &)            = delete;
  String &operator=(String const &) = delete;

  String(String &&other) :
      memory_{std::move(other.memory_)}, size_{other.size_}
  {
    other.memory_.allocator = static_storage_allocator;
    other.memory_.handle    = EMPTY_STRING;
    other.size_             = 0;
  }

  String &operator=(String &&other)
  {
    std::swap(memory_, other.memory_);
    std::swap(size_, other.size_);

    return *this;
  }

  char const *c_str() const
  {
    return data();
  }

  Pointer data() const
  {
    return static_cast<char const *>(memory_.handle);
  }

  Size size() const
  {
    return size_;
  }

  Iterator begin() const
  {
    return data();
  }

  Iterator end() const
  {
    return begin() + size();
  }

  bool is_empty() const
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

  bool starts_with(std::string_view other) const
  {
    if (other.size() > size_)
    {
      return false;
    }

    return std::memcmp(data(), other.data(), other.size()) == 0;
  }

  bool starts_with(char const *other) const
  {
    return starts_with(std::string_view{other});
  }

  bool starts_with(String const &other) const
  {
    return starts_with(other.view());
  }

  bool starts_with(char c) const
  {
    return size_ > 0 && data()[0] == c;
  }

  bool ends_with(std::string_view other) const
  {
    if (other.size() > size_)
    {
      return false;
    }

    return std::memcmp(data() + (size_ - other.size()), other.data(), other.size()) == 0;
  }

  bool ends_with(char const *other) const
  {
    return ends_with(std::string_view{other});
  }

  bool ends_with(String const &other) const
  {
    return ends_with(other.view());
  }

  bool ends_with(char c) const
  {
    return size_ > 0 && data()[size_ - 1] == c;
  }

  std::string_view view() const
  {
    return std::string_view{data(), size_};
  }

  Span<char const> span() const
  {
    return Span<char const>{data(), size()};
  }

  bool operator==(std::string_view other) const
  {
    if (size() != other.size())
    {
      return false;
    }

    return std::memcmp(data(), other.data(), size()) == 0;
  }

  bool operator==(char const *other) const
  {
    return *this == std::string_view{other};
  }

  bool operator==(String const &other) const
  {
    return *this == std::string_view{other};
  }

  bool operator!=(std::string_view other) const
  {
    return !(*this == other);
  }

  bool operator!=(char const *other) const
  {
    return *this != std::string_view{other};
  }

  bool operator!=(String const &other) const
  {
    return !(*this == other);
  }

  operator std::string_view() const
  {
    return std::string_view{data(), size()};
  }

  operator CStringView() const
  {
    return CStringView{data(), size()};
  }

  Result<String, AllocError> copy(Allocator allocator) const
  {
    TRY_OK(memory, mem::allocate(allocator, size_ + 1));

    std::memcpy(memory.handle, memory_.handle, size_ + 1);

    return Ok(String{ReadOnlyMemory{std::move(memory)}, size_});
  }

  ReadOnlyMemory memory_;
  Size           size_ = 0;
};

inline namespace literals
{

inline String operator""_str(char const *string_literal, size_t str_size)
{
  return String{ReadOnlyMemory{static_storage_allocator, string_literal}, str_size};
}

}        // namespace literals

namespace string
{

inline Result<String, AllocError> make(Allocator allocator, std::string_view str)
{
  TRY_OK(memory, mem::allocate(allocator, str.size() + 1));

  std::memcpy(memory.handle, str.data(), str.size());

  static_cast<char *>(memory.handle)[str.size()] = '\0';

  return Ok(String{ReadOnlyMemory{std::move(memory)}, str.size()});
}

inline String make_static(std::string_view str)
{
  return String{ReadOnlyMemory{static_storage_allocator, str.data()}, str.size()};
}

namespace rc
{

inline Rc<std::string_view> make_static_view(std::string_view str)
{
  Manager manager{static_storage_manager};
  manager.ref();
  return Rc<std::string_view>{std::move(str), std::move(manager)};
}

}        // namespace rc

template <typename Glue, typename A, typename B, typename... S>
Result<String, AllocError> join(Allocator allocator, Glue const &glue,
                                A const &a, B const &b, S const &...s)
{
  static_assert(std::is_convertible_v<Glue const &, std::string_view>);
  static_assert(std::is_convertible_v<A const &, std::string_view>);
  static_assert(std::is_convertible_v<B const &, std::string_view> && (std::is_convertible_v<S const &, std::string_view> && ...));

  std::string_view views[] = {std::string_view{a}, std::string_view{b}, std::string_view{s}...};
  size_t           nviews  = std::size(views);

  std::string_view glue_v{glue};

  size_t str_size = 0;

  {
    size_t index = 0;

    for (std::string_view v : views)
    {
      str_size += v.size();

      if (index != nviews - 1)
      {
        str_size += glue_v.size();
      }

      index++;
    }
  }

  // with null terminator
  size_t memory_size = str_size + 1;

  TRY_OK(memory, mem::allocate(allocator, memory_size));

  char *str = static_cast<char *>(memory.handle);

  {
    size_t index      = 0;
    size_t view_index = 0;

    for (std::string_view v : views)
    {
      std::memcpy(str + index, v.data(), v.size());
      index += v.size();

      if (view_index != nviews - 1)
      {
        std::memcpy(str + index, glue_v.data(), glue_v.size());
        index += glue_v.size();
      }

      view_index++;
    }
  }

  str[str_size] = '\0';

  return Ok(String{ReadOnlyMemory{std::move(memory)}, str_size});
}

template <typename Glue, typename T>
Result<String, AllocError> join(Allocator allocator, Glue const &glue,
                                Span<T> strings)
{
  static_assert(std::is_convertible_v<T &, std::string_view>);
  static_assert(std::is_convertible_v<Glue const &, std::string_view>);

  size_t           size     = 0;
  size_t           nstrings = strings.size();
  std::string_view glue_v{glue};

  {
    size_t str_index = 0;

    for (T const &string : strings)
    {
      size += std::string_view{string}.size();
      if (str_index != nstrings - 1)
      {
        size += glue_v.size();
      }
      str_index++;
    }
  }

  size_t memory_size = size + 1;

  TRY_OK(memory, mem::allocate(allocator, memory_size));

  char *out = static_cast<char *>(memory.handle);

  {
    size_t index     = 0;
    size_t str_index = 0;

    for (T const &string : strings)
    {
      std::string_view view{string};
      std::memcpy(out + index, view.data(), view.size());
      index += view.size();

      if (str_index != nstrings - 1)
      {
        std::memcpy(out + index, glue_v.data(), glue_v.size());
        index += glue_v.size();
      }

      str_index++;
    }
  }

  out[size] = '\0';

  return Ok(String{ReadOnlyMemory{std::move(memory)}, size});
}

inline Result<String, AllocError> upper(Allocator allocator, std::string_view str)
{
  TRY_OK(memory, mem::allocate(allocator, str.size() + 1));

  char *out = static_cast<char *>(memory.handle);

  for (size_t i = 0; i < str.size(); i++)
  {
    out[i] = std::toupper(str[i]);
  }

  out[str.size()] = '\0';

  return Ok(String{ReadOnlyMemory{std::move(memory)}, str.size()});
}

inline Result<String, AllocError> lower(Allocator allocator, std::string_view str)
{
  size_t size = str.size();

  TRY_OK(memory, mem::allocate(allocator, size + 1));

  char *out = static_cast<char *>(memory.handle);

  for (size_t i = 0; i < size; i++)
  {
    out[i] = std::tolower(str[i]);
  }

  out[size] = '\0';

  return Ok(String{ReadOnlyMemory{std::move(memory)}, size});
}

// TODO(lamarrr):
// void split(allocator, string, delimeter, callback);
//
}        // namespace string

STX_END_NAMESPACE
