/**
 * @file common_test.cc
 * @author Basit Ayantunde <rlamarrr@gmail.com>
 * @version  0.1.0
 * @date 2020-04-16
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

#include "stx/common.h"

#include <memory>
#include <vector>

using namespace std;  // NOLINT
using namespace stx;  // NOLINT

struct Immovable {
  Immovable() = delete;
  Immovable(Immovable&&) = delete;
  Immovable& operator=(Immovable&&) = delete;
  Immovable(Immovable const&) = default;
  Immovable& operator=(Immovable const&) = default;
};

struct Movable {
  Movable() = default;
  Movable(Movable&&) = default;
  Movable& operator=(Movable&&) = default;
  Movable(Movable const&) = delete;
  Movable& operator=(Movable const&) = delete;
};

static_assert(!std::is_default_constructible_v<Immovable>);
static_assert(std::is_default_constructible_v<Movable>);

struct WithRv {
  int v;
  WithRv(int&& vv) : v{std::forward<int>(vv)} {}
};

struct NotWithRv {
  int v;
  NotWithRv(int& vv) : v{vv} {}
};

static_assert(std::is_convertible_v<int const, int>);
static_assert(std::is_convertible_v<int const, int>);
static_assert(std::is_convertible_v<int const, int>);

static_assert(std::is_convertible_v<int, WithRv>);
static_assert(!std::is_convertible_v<int const, WithRv>);
static_assert(!std::is_convertible_v<int&, WithRv>);

static_assert(!std::is_convertible_v<int const, NotWithRv>);
static_assert(!std::is_convertible_v<int const, NotWithRv>);
static_assert(std::is_convertible_v<int&, NotWithRv>);

// is the usage of std::is_convertible_v right?

static_assert(!std::is_copy_constructible_v<unique_ptr<int>>);
static_assert(std::is_copy_constructible_v<int>);
static_assert(std::is_copy_constructible_v<int const>);
static_assert(std::is_copy_constructible_v<vector<int>>);
static_assert(std::is_copy_constructible_v<vector<int> const>);

static_assert(std::is_invocable_v<int (*&)(int&&), int>);
static_assert(!std::is_invocable_v<int (*&)(int&&), int&>);
static_assert(std::is_invocable_v<int (*&)(int&&), int&&>);
