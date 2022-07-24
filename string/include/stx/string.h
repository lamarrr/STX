
#pragma once

#include <cctype>
#include <cstddef>
#include <cstring>
#include <string_view>
#include <utility>

#include "stx/allocator.h"
#include "stx/config.h"
#include "stx/mem.h"
#include "stx/option.h"
#include "stx/rc.h"
#include "stx/span.h"

STX_BEGIN_NAMESPACE

constexpr char const EMPTY_STRING[] = "";

#define STX_ENSURE(condition, error_message)

using StringView = std::string_view;
using StaticStringView = std::string_view;

// meaning, i want to share this, but I don't care about it's source or any
// allocation operations.
// I just want to be able to read the string as long as I have this Rc.
//
// Can be copied and shared across threads.
//
//
using RcStringView = Rc<StringView>;

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
struct String {
  String() : memory_{static_storage_allocator, EMPTY_STRING}, size_{0} {}

  String(ReadOnlyMemory memory, size_t size)
      : memory_{std::move(memory)}, size_{size} {}

  String(String const&) = delete;
  String& operator=(String const&) = delete;

  String(String&& other)
      : memory_{std::move(other.memory_)}, size_{other.size_} {
    other.memory_.allocator = static_storage_allocator;
    other.memory_.handle = EMPTY_STRING;
    other.size_ = 0;
  }

  String& operator=(String&& other) {
    std::swap(memory_, other.memory_);
    std::swap(size_, other.size_);

    return *this;
  }

  char const* c_str() const { return data(); }

  char const* data() const { return static_cast<char const*>(memory_.handle); }

  size_t size() const { return size_; }

  char const* begin() const { return data(); }

  char const* end() const { return begin() + size(); }

  bool is_empty() const { return size_ == 0; }

  char const& operator[](size_t index) const { return span()[index]; }

  Option<Ref<char const>> at(size_t index) const { return span().at(index); }

  bool starts_with(StringView other) const {
    if (other.size() > size_) return false;

    return std::memcmp(data(), other.data(), other.size()) == 0;
  }

  bool starts_with(String const& other) const {
    return starts_with(other.view());
  }

  bool starts_with(char c) const { return size_ > 0 && data()[0] == c; }

  bool ends_with(StringView other) const {
    if (other.size() > size_) return false;

    return std::memcmp(data() + (size_ - other.size()), other.data(),
                       other.size()) == 0;
  }

  bool ends_with(String const& other) const { return ends_with(other.view()); }

  bool ends_with(char c) const { return size_ > 0 && data()[size_ - 1] == c; }

  StringView view() const { return StringView{data(), size_}; }

  Span<char const> span() const { return Span<char const>{data(), size()}; }

  bool equals(StringView other) const {
    if (size() != other.size()) return false;

    return std::memcmp(data(), other.data(), size()) == 0;
  }

  bool equals(String const& other) const { return equals(other.view()); }

  bool not_equals(StringView other) const { return !equals(other); }

  bool not_equals(String const& other) const { return !equals(other); }

  bool operator==(String const& other) const { return equals(other); }

  bool operator!=(String const& other) const { return not_equals(other); }

  ReadOnlyMemory memory_;
  size_t size_ = 0;
};

namespace string {

inline Result<String, AllocError> make(Allocator allocator, StringView str) {
  TRY_OK(memory, mem::allocate(allocator, str.size() + 1));

  std::memcpy(memory.handle, str.data(), str.size());

  static_cast<char*>(memory.handle)[str.size()] = '\0';

  ReadOnlyMemory str_memory{std::move(memory)};

  return Ok(String{std::move(str_memory), str.size()});
}

inline String make_static(StaticStringView str) {
  return String{ReadOnlyMemory{static_storage_allocator, str.data()},
                str.size()};
}

namespace rc {

inline RcStringView make_static_view(StaticStringView str) {
  Manager manager{static_storage_manager};
  manager.ref();
  return Rc<StringView>{std::move(str), std::move(manager)};
}

}  // namespace rc

template <typename Glue, typename A, typename B, typename... S>
Result<String, AllocError> join(Allocator allocator, Glue const& glue,
                                A const& a, B const& b, S const&... s) {
  static_assert(std::is_convertible_v<Glue const&, StringView> &&
                std::is_convertible_v<A const&, StringView> &&
                std::is_convertible_v<B const&, StringView> &&
                (std::is_convertible_v<S const&, StringView> && ...));

  StringView views[] = {StringView{a}, StringView{b}, StringView{s}...};
  size_t nviews = std::size(views);

  StringView glue_v{glue};

  size_t str_size = 0;

  {
    size_t index = 0;

    for (StringView v : views) {
      str_size += v.size();

      if (index != nviews - 1) {
        str_size += glue_v.size();
      }

      index++;
    }
  }

  // with null terminator
  size_t memory_size = str_size + 1;

  TRY_OK(memory, mem::allocate(allocator, memory_size));

  char* str = static_cast<char*>(memory.handle);

  str[str_size] = '\0';

  {
    size_t index = 0;
    size_t view_index = 0;

    for (StringView v : views) {
      std::memcpy(str + index, v.begin(), v.size());
      index += v.size();

      if (view_index != nviews - 1) {
        std::memcpy(str + index, glue_v.begin(), glue_v.size());
        index += glue_v.size();
      }

      view_index++;
    }
  }

  return Ok(String{ReadOnlyMemory{std::move(memory)}, str_size});
}

//        split(allocator, del, ...);
//

}  // namespace string

STX_END_NAMESPACE
