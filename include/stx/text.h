#pragma once
#include <cinttypes>
#include <string_view>

#include "stx/config.h"

STX_BEGIN_NAMESPACE

/// gets the unicode codepoint at iter and then advances iter to the next codepoint
///
constexpr uint32_t utf8_next(uint8_t const *&iter)
{
  if ((*iter & 0xF8) == 0xF0)
  {
    uint32_t c1 = *iter;
    iter++;
    uint32_t c2 = *iter;
    iter++;
    uint32_t c3 = *iter;
    iter++;
    uint32_t c4 = *iter;
    iter++;
    return c1 << 24 | c2 << 16 | c3 << 8 | c4;
  }
  else if ((*iter & 0xF0) == 0xE0)
  {
    uint32_t c1 = *iter;
    iter++;
    uint32_t c2 = *iter;
    iter++;
    uint32_t c3 = *iter;
    iter++;
    return c1 << 16 | c2 << 8 | c3;
  }
  else if ((*iter & 0xE0) == 0xC0)
  {
    uint32_t c1 = *iter;
    iter++;
    uint32_t c2 = *iter;
    iter++;
    return c1 << 8 | c2;
  }
  else
  {
    uint32_t c1 = *iter;
    iter++;
    return c1;
  }
}

STX_END_NAMESPACE
