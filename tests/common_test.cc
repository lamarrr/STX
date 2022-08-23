/**
 * @file common_test.cc
 * @author Basit Ayantunde <rlamarrr@gmail.com>
 * @date 2020-04-16
 *
 * @copyright MIT License
 *
 * Copyright (c) 2020-2022 Basit Ayantunde
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
