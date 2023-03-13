/**
 * @file constexpr_test.cc
 * @author Basit Ayantunde <rlamarrr@gmail.com>
 * @date 2020-06-05
 *
 * @copyright MIT License
 *
 * Copyright (c) 2020-2022 Basit Ayantunde
 *
 */

#include "stx/option.h"
#include "stx/result.h"

#if STX_OPTION_IS_CONSTEXPR && STX_RESULT_IS_CONSTEXPR        // check if it'll work

using stx::Ok, stx::Err, stx::Result;
using stx::Some, stx::None, stx::Option;

constexpr auto divide(int x, int y) -> Option<int>
{
  if (y == 0)
    return None;
  return Some(x / y);
}

static_assert(divide(56, 1).unwrap_or_default() == 56);
static_assert(divide(56, 0).unwrap_or_default() == 0);

static_assert(divide(56, 10).match([](int v) { return v; },
                                   []() { return -1; }) == 5);
static_assert(divide(56, 0).match([](int v) { return v; },
                                  []() { return -1; }) == -1);

enum class Error
{
  Range
};

constexpr auto add(int8_t x, int8_t y) -> Result<int8_t, Error>
{
  int16_t acc = x;
  acc += y;

  if (acc > 127)
    return Err(Error::Range);
  return Ok((int8_t) acc);
}

static_assert(add(0, 10).unwrap_or_default() == 10);
static_assert(add(127, 1).unwrap_or_default() == 0);
static_assert(add(56, 10).match([](int8_t v) { return v; },
                                [](Error) { return -1; }) == 66);
static_assert(add(56, 100).match([](int8_t v) { return v; },
                                 [](Error) { return -1; }) == -1);

#endif