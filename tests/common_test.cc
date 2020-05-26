/**
 * @file common_test.cc
 * @author Basit Ayantunde <rlamarrr@gmail.com>
 * @brief
 * @version  0.1
 * @date 2020-04-16
 *
 * @copyright Copyright (c) 2020
 *
 */

#include "stx/common.h"

#include <memory>
#include <vector>

using namespace std;  // NOLINT
using namespace stx;  // NOLINT

template <typename T, typename Cmp>
concept Same = std::is_same_v<T, Cmp>;

static_assert(Derefable<int*> && MutDerefable<int*>);
static_assert(Derefable<int* const> && MutDerefable<int* const> &&
              ConstDerefable<int* const>);
static_assert(Derefable<int const*> &&
              (!MutDerefable<int const*>)&&ConstDerefable<int const*>);
static_assert(Derefable<const int* const> && ConstDerefable<const int* const> &&
              !MutDerefable<const int* const>);

static_assert(Derefable<vector<int>*>);
static_assert(Derefable<vector<int>* const>);
static_assert(Derefable<const vector<int>*>);
static_assert(Derefable<const vector<int>* const>);

static_assert(Derefable<vector<int>::iterator>);
static_assert(Derefable<vector<int>::const_iterator>);
static_assert(Derefable<vector<int>::reverse_iterator>);
static_assert(Derefable<vector<int>::const_reverse_iterator>);

static_assert(!Derefable<int>);
static_assert(!Derefable<vector<int>>);

static_assert(same_as<Deref<int*>, std::reference_wrapper<int>>);
static_assert(same_as<Deref<int const*>, std::reference_wrapper<int const>>);
static_assert(same_as<Deref<int* const>, std::reference_wrapper<int>>);
static_assert(
    same_as<Deref<int const* const>, std::reference_wrapper<int const>>);

static_assert(same_as<Deref<typename vector<int>::iterator>,
                      std::reference_wrapper<int>>);

static_assert(same_as<Deref<typename vector<int>::const_iterator>,
                      std::reference_wrapper<int const>>);

static_assert(!Pointer<int>);
static_assert(Pointer<int*>);

static_assert(OutputPointer<int*>);
static_assert(OutputPointer<int* const>);
static_assert(OutputPointer<int**>);
static_assert(OutputPointer<int const**>);
static_assert(OutputPointer<int** const>);
static_assert(OutputPointer<int* const** const>);
static_assert(!OutputPointer<int const*>);
static_assert(!OutputPointer<int const* const>);
static_assert(!OutputPointer<int* const*>);
static_assert(!OutputPointer<int const* const*>);
static_assert(!OutputPointer<int* const* const>);
static_assert(!OutputPointer<int* const* const* const>);

static_assert(InputPointer<int*>);
static_assert(InputPointer<int* const>);
static_assert(InputPointer<int**>);
static_assert(InputPointer<int const**>);
static_assert(InputPointer<int** const>);
static_assert(InputPointer<int* const** const>);
static_assert(InputPointer<int const*>);
static_assert(InputPointer<int const* const>);
static_assert(InputPointer<int* const*>);
static_assert(InputPointer<int const* const*>);
static_assert(InputPointer<int* const* const>);
static_assert(InputPointer<int* const* const* const>);

static_assert(!UnaryDerefableSmartPointer<int>);
static_assert(!UnaryDerefableSmartPointer<int*>);
static_assert(!UnaryDerefableSmartPointer<typename vector<int>::iterator>);
static_assert(UnaryDerefableSmartPointer<unique_ptr<int>>);
// you have to first lock the weak pointer
static_assert(!UnaryDerefableSmartPointer<weak_ptr<int>>);
static_assert(UnaryDerefableSmartPointer<shared_ptr<int>>);

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

static_assert(Swappable<int>);
static_assert(!Swappable<int const>);
static_assert(Swappable<vector<int>>);
static_assert(!Swappable<vector<int> const>);
static_assert(!Swappable<Immovable>);
static_assert(!Swappable<Immovable const>);
static_assert(Swappable<Movable>);
static_assert(!Swappable<Movable const>);

static_assert(!default_constructible<Immovable>);
static_assert(default_constructible<Movable>);

static_assert(Same<MutDeref<int*>, reference_wrapper<int>>);
static_assert(Same<ConstDeref<int*>, reference_wrapper<int const>>);
static_assert(Same<MutDeref<int* const>, reference_wrapper<int>>);
static_assert(Same<ConstDeref<int* const>, reference_wrapper<int const>>);
static_assert(Same<MutDeref<vector<int>::iterator>, reference_wrapper<int>>);
static_assert(
    Same<ConstDeref<vector<int>::iterator>, reference_wrapper<int const>>);
static_assert(Same<ConstDeref<typename vector<int>::const_iterator>,
                   reference_wrapper<int const>>);

struct WithRv {
  int v;
  WithRv(int&& vv) : v{std::forward<int>(vv)} {}
};

struct NotWithRv {
  int v;
  NotWithRv(int& vv) : v{vv} {}
};

static_assert(convertible_to<int const, int>);
static_assert(convertible_to<int const, int>);
static_assert(convertible_to<int const, int>);

static_assert(convertible_to<int, WithRv>);
static_assert(!convertible_to<int const, WithRv>);
static_assert(!convertible_to<int&, WithRv>);

static_assert(!convertible_to<int const, NotWithRv>);
static_assert(!convertible_to<int const, NotWithRv>);
static_assert(convertible_to<int&, NotWithRv>);

// is the usage of convertible right?

static_assert(!copy_constructible<unique_ptr<int>>);
static_assert(copy_constructible<int>);
static_assert(copy_constructible<int const>);
static_assert(copy_constructible<vector<int>>);
static_assert(copy_constructible<vector<int> const>);

static_assert(invocable<int (*&)(int&&), int>);
static_assert(!invocable<int (*&)(int&&), int&>);
static_assert(invocable<int (*&)(int&&), int&&>);

static_assert(Same<void, std::invoke_result_t<void (*)(int), int>>);
