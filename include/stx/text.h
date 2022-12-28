#pragma once
#include <cinttypes>
#include <string_view>

#include "stx/config.h"

STX_BEGIN_NAMESPACE

/// a utf-8 text iterator
struct TextIterator {
  uint8_t const* iter = nullptr;

  constexpr TextIterator(uint8_t const* str) : iter{str} {}

  constexpr TextIterator(char const* str)
      : iter{reinterpret_cast<uint8_t*>(str)} {}

  constexpr TextIterator(std::string_view str) : TextIterator{str.data()} {}

  uint32_t next() {
    if ((*iter & 0xF8) == 0xF0) {
      uint32_t c1 = *iter;
      iter++;
      uint32_t c2 = *iter;
      iter++;
      uint32_t c3 = *iter;
      iter++;
      uint32_t c4 = *iter;
      iter++;
      return c1 << 24 | c2 << 16 | c3 << 8 | c4;
    } else if ((*iter & 0xF0) == 0xE0) {
      uint32_t c1 = *iter;
      iter++;
      uint32_t c2 = *iter;
      iter++;
      uint32_t c3 = *iter;
      iter++;
      return c1 << 16 | c2 << 8 | c3;
    } else if ((*iter & 0xE0) == 0xC0) {
      uint32_t c1 = *iter;
      iter++;
      uint32_t c2 = *iter;
      iter++;
      return c1 << 8 | c2;
    } else {
      uint32_t c1 = *iter;
      iter++;
      return c1;
    }
  }
};

constexpr bool operator==(TextIterator const& a, uint8_t const* b) {
  return a.iter == b;
}

constexpr bool operator!=(TextIterator const& a, uint8_t const* b) {
  return a.iter != b;
}

constexpr bool operator==(TextIterator const& a, char const* b) {
  return a.iter == reinterpret_cast<uint8_t*>(b);
}

constexpr bool operator!=(TextIterator const& a, char const* b) {
  return a.iter != reinterpret_cast<uint8_t*>(b);
}

constexpr bool operator==(uint8_t const* a, TextIterator const& b) {
  return a == b.iter;
}

constexpr bool operator!=(uint8_t const* a, TextIterator const& b) {
  return a != b.iter;
}

constexpr bool operator==(char const* a, TextIterator const& b) {
  return reinterpret_cast<uint8_t*>(a) == b.iter;
}

constexpr bool operator!=(char const* a, TextIterator const& b) {
  return reinterpret_cast<uint8_t*>(a) != b.iter;
}

constexpr bool operator==(TextIterator const& a, TextIterator const& b) {
  return a.iter == b.iter;
}

constexpr bool operator!=(TextIterator const& a, TextIterator const& b) {
  return a.iter != b.iter;
}

STX_END_NAMESPACE
