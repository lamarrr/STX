/**
 * @file constexpr_test.cc
 * @author Basit Ayantunde <rlamarrr@gmail.com>
 * @date 2020-06-05
 *
 * @copyright MIT License
 *
 * Copyright (c) 2020 Basit Ayantunde
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */

#include "stx/option.h"
#include "stx/result.h"

using stx::Ok, stx::Err, stx::Result;
using stx::Some, stx::None, stx::Option;

struct PartlyNonTrivial {
  PartlyNonTrivial();                        // nontrivial
  PartlyNonTrivial(PartlyNonTrivial const&); // nontrivial
  PartlyNonTrivial(PartlyNonTrivial&&);      // nontrivial
  PartlyNonTrivial& operator=(PartlyNonTrivial&&); // nontrivial
  PartlyNonTrivial& operator=(PartlyNonTrivial const&) = default; // trivial
};

static_assert(std::is_trivially_copyable_v<Option<int>>);
static_assert(!std::is_trivially_copyable_v<Option<PartlyNonTrivial>>);
static_assert(!std::is_trivially_move_assignable_v<Option<PartlyNonTrivial>>);
static_assert(!std::is_trivially_move_constructible_v<Option<PartlyNonTrivial>>);
static_assert(std::is_trivially_copy_assignable_v<Option<PartlyNonTrivial>>);

template <typename T>
constexpr auto copy(T const& arg) {
  return arg;
}

template <typename T>
constexpr auto move(T&& arg) {
  return std::move(arg);
}

constexpr auto divide(int x, int y) -> Option<int> {
  if (y == 0) return None;
  return Some(x / y);
}

static_assert(divide(56, 1).unwrap_or_default() == 56);
static_assert(
    Option<int>(copy(divide(56, 1))).unwrap_or_default() ==
    56); // copy ctor


static_assert(
    Option<int>(move(divide(56, 1))).unwrap_or_default() ==
    56); // move ctor
static_assert(divide(56, 0).unwrap_or_default() == 0);

static_assert(divide(56, 10).match([](int v) { return v; },
                                   []() { return -1; }) == 5);
static_assert(divide(56, 0).match([](int v) { return v; },
                                  []() { return -1; }) == -1);

struct NonTrivial {
  constexpr NonTrivial() {}
  constexpr NonTrivial(NonTrivial&&) {}
  constexpr NonTrivial(NonTrivial const&) {}
  constexpr NonTrivial& operator=(NonTrivial const&) { return *this; }
  constexpr NonTrivial& operator=(NonTrivial&&) { return *this; }
  constexpr operator bool() const { return true; }
};
static_assert(NonTrivial{});
static_assert(!Option<NonTrivial>{});
static_assert(Option(Some(NonTrivial{})).unwrap());

#if STX_OPTION_IS_CONSTEXPR && STX_RESULT_IS_CONSTEXPR  // check if it'll work
static_assert((copy(Option(Some(NonTrivial{}))).unwrap()));
static_assert((move(Option(Some(NonTrivial{}))).unwrap()));

enum class Error { Range };

constexpr auto add(int8_t x, int8_t y) -> Result<int8_t, Error> {
  int16_t acc = x;
  acc += y;

  if (acc > 127) return Err(Error::Range);
  return Ok((int8_t)acc);
}

static_assert(add(0, 10).unwrap_or_default() == 10);
static_assert(add(127, 1).unwrap_or_default() == 0);
static_assert(add(56, 10).match([](int8_t v) { return v; },
                                [](Error) { return -1; }) == 66);
static_assert(add(56, 100).match([](int8_t v) { return v; },
                                 [](Error) { return -1; }) == -1);

#endif
