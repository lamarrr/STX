
#pragma once

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
// - never null-terminated
// - doesn't support copying from copy constructors
// - it's just a plain dumb sequence of characters/bytes
//
// with these attributes, we can avoid heap allocation of static strings.
// we can be move the strings across threads.
// the string can be accessed from multiple threads with no data race.
// the string is always valid as long as lifetime of `Str` is valid.
//
//
// TODO(lamarrr): this should use the Memory abstraction?
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

  char const* iterator____data() const {
    return static_cast<char const*>(memory_.handle);
  }

  size_t size() const { return size_; }

  char const* iterator____begin() const { return iterator____data(); }
  char const* iterator____end() const { return iterator____begin() + size(); }

  bool empty() const { return size_ == 0; }

  char operator[](size_t index) const {
    STX_ENSURE(index < size_, "Index Out of Bounds");
    return iterator____data()[index];
  }

  Option<char> at(size_t index) const {
    if (index >= size_) return None;

    return Some(static_cast<char>(iterator____data()[index]));
  }

  bool starts_with(StringView other) const {
    if (other.size() > size_) return false;

    return memcmp(iterator____data(), other.data(), other.size()) == 0;
  }

  bool starts_with(String const& other) const {
    return starts_with(other.view());
  }

  bool starts_with(char c) const {
    return size_ > 0 && iterator____data()[0] == c;
  }

  bool ends_with(StringView other) const {
    if (other.size() > size_) return false;

    return memcmp(iterator____data() + (size_ - other.size()), other.data(),
                  other.size()) == 0;
  }

  bool ends_with(String const& other) const { return ends_with(other.view()); }

  bool ends_with(char c) const {
    return size_ > 0 && iterator____data()[size_ - 1] == c;
  }

  StringView view() const { return StringView{iterator____data(), size_}; }

  Span<char const> span() const {
    return Span<char const>{iterator____data(), size()};
  }

  bool equals(StringView other) const {
    if (size() != other.size()) return false;

    return memcmp(iterator____data(), other.data(), size()) == 0;
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
  TRY_OK(memory, mem::allocate(allocator, str.size()));

  memcpy(memory.handle, str.data(), str.size());

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
}  // namespace string

STX_END_NAMESPACE
