/**
 * @file option_test.cc
 * @author Basit Ayantunde <rlamarrr@gmail.com>
 * @date 2020-04-16
 *
 * @copyright MIT License
 *
 * Copyright (c) 2020-2021 Basit Ayantunde
 *
 */

#include "stx/option.h"

#include <algorithm>
#include <memory>
#include <numeric>
#include <vector>

#include "gtest/gtest.h"
#include "stx/panic.h"

using namespace std;  // NOLINT
using namespace stx;  // NOLINT

// TODO(lamarrr) test against all methods

template <size_t ID>
struct MoveOnly {
  explicit MoveOnly(int) {}
  // no implicit defaul construction
  MoveOnly() { stx::panic("\t>> MoveOnly<" + to_string(ID) + ">::construct"); }
  MoveOnly(MoveOnly const&) {
    stx::panic("\t>> MoveOnly<" + to_string(ID) + ">::copy_construct called");
  }
  MoveOnly(MoveOnly&&) = default;
  MoveOnly& operator=(MoveOnly const&) {
    stx::panic("\t>> MoveOnly<" + to_string(ID) + ">::copy_assign called");
    return *this;
  }
  MoveOnly& operator=(MoveOnly&&) = default;
  ~MoveOnly() = default;

  void done() const {
    // cout << "\t>> MoveOnly<" << to_string(ID) << "> Done!" << std::endl;
  }

  bool operator==(MoveOnly const&) const { return true; }
  bool operator!=(MoveOnly const&) const { return false; }
};

template <size_t id>
MoveOnly<id> make_mv() {
  return MoveOnly<id>(id);
}

struct NonTrivial {
  NonTrivial(NonTrivial&&) {}

  NonTrivial& operator=(NonTrivial&&) { return *this; }
};

static_assert(std::is_swappable_v<MoveOnly<0>>);
static_assert(stx::equality_comparable<MoveOnly<0>>);

static_assert(std::is_trivially_move_constructible_v<Option<int>>);
static_assert(std::is_trivially_move_assignable_v<Option<int>>);
static_assert(!std::is_trivially_move_constructible_v<NonTrivial>);
static_assert(!std::is_trivially_move_assignable_v<NonTrivial>);

struct FnMut {
  int call_times;
  FnMut() : call_times{0} {}
  int operator()(int&& x) {
    call_times++;
    return x;
  }
  int operator()(int&& x) const { return x; }
};

struct FnConst {
  int operator()(int&& x) const { return x; }
};

TEST(OptionTest, Misc) {
  EXPECT_EQ(Option<Option<int>>(Some(Option(Some(899)))).unwrap().unwrap(),
            899);
}

TEST(OptionTest, ObjectConstructionTest) {
  Option<int> a = None;
  Option b = Some(89);
  EXPECT_DEATH_IF_SUPPORTED(move(a).unwrap(), ".*");
  EXPECT_NO_THROW(move(b).unwrap());
  EXPECT_EQ(Option(Some(89)).unwrap(), 89);

  auto fn_a = []() -> Option<MoveOnly<0>> {
    return Some(make_mv<0>());  // NOLINT
  };
  EXPECT_NO_THROW(auto a_ = fn_a());
  auto fn_b = []() -> Option<MoveOnly<1>> { return None; };
  EXPECT_NO_THROW(auto b_ = fn_b());

  auto d = fn_a();
  d = Some(make_mv<0>());
  d = None;
  d = Some(make_mv<0>());
}

TEST(OptionTest, CopyConstructionTest) {
  {
    Option<int> a = None;
    Option<int> b = a.copy();
    EXPECT_EQ(a, b);

    Option<int> c = Some(98);
    b = c.copy();
    EXPECT_EQ(b, c);
    EXPECT_NE(a, c);
    EXPECT_NE(a, b);

    Option<vector<int>> d = None;
    Option<vector<int>> e = d.copy();
    EXPECT_EQ(d, e);

    Option f = Some(vector{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15});
    e = f.copy();
    EXPECT_EQ(e, f);
    EXPECT_NE(d, e);
    EXPECT_NE(d, f);
  }

  {
    Option<int> a = None;
    Option<int> b = a;
    EXPECT_EQ(a, b);

    Option<int> c = Some(98);
    b = c;
    EXPECT_EQ(b, c);
    EXPECT_NE(a, c);
    EXPECT_NE(a, b);

    Option<vector<int>> d = None;
    Option<vector<int>> e = d;
    EXPECT_EQ(d, e);

    Option f = Some(vector{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15});
    e = f;
    EXPECT_EQ(e, f);
    EXPECT_NE(d, e);
    EXPECT_NE(d, f);
  }
}

TEST(OptionTest, ObjectForwardingTest) {
  auto fn_a = []() -> Option<MoveOnly<0>> {
    return Some(make_mv<0>());  // NOLINT
  };
  EXPECT_NO_THROW(fn_a().unwrap());
  auto fn_b = []() -> Option<unique_ptr<int[]>> {
    return Some(make_unique<int[]>(1024));
  };
  EXPECT_NO_THROW(fn_b().unwrap());

  Option g = Some(vector{1, 2, 3, 4, 5});

  g = Some(vector{5, 6, 7, 8, 9});

  EXPECT_EQ(g, Some(vector{5, 6, 7, 8, 9}));

  g = None;

  EXPECT_EQ(g, None);

  g = Some(vector{1, 2, 3, 4, 5});

  EXPECT_EQ(g, Some(vector{1, 2, 3, 4, 5}));

  g = None;

  EXPECT_EQ(g, None);
}

TEST(OptionTest, Equality) {
  constexpr Option<int> h{};
  static_assert(h == None);

  // EXPECT_NE(Some(0), None);
  EXPECT_EQ(Some(90), Some(90));
  EXPECT_NE(Some(90), Some(70));
  // EXPECT_NE(Some<Option<int>>(None), None);
  EXPECT_EQ(None, None);
  EXPECT_EQ(Option(Some(90)), Some(90));
  EXPECT_NE(Option(Some(90)), Some(70));
  EXPECT_EQ(Option(Some(90)), Option(Some(90)));
  EXPECT_NE(Option(Some(90)), Option(Some(20)));
  EXPECT_NE(Option(Some(90)), None);
  EXPECT_EQ(Option<int>(None), None);
  EXPECT_NE(Option<Option<int>>(Some(Option<int>(None))), None);

  // EXPECT_NE(None, Some(0));
  // EXPECT_NE(None, Some<Option<int>>(None));
  EXPECT_EQ(Some(90), Option(Some(90)));
  EXPECT_NE(Some(70), Option(Some(90)));
  EXPECT_NE(None, Option(Some(90)));
  EXPECT_EQ(None, Option<int>(None));
  EXPECT_NE(None, Option<Option<int>>(Some(Option<int>(None))));

  int const x = 909'909;
  int y = 909'909;

  EXPECT_EQ(some_ref(x), Some(909'909));
  EXPECT_EQ(some_ref(y), Some(909'909));

  EXPECT_EQ(Some(909'909), some_ref(y));
  EXPECT_EQ(Some(909'909), some_ref(y));

  EXPECT_EQ(Option(Some(909'909)), some_ref(x));
  EXPECT_EQ(Option(Some(909'909)), some_ref(y));
  EXPECT_NE(Option(Some(101'101)), some_ref(x));
  EXPECT_NE(Option(Some(101'101)), some_ref(y));

  EXPECT_EQ(some_ref(x), Option(Some(909'909)));
  EXPECT_EQ(some_ref(y), Option(Some(909'909)));
  EXPECT_NE(some_ref(x), Option(Some(101'101)));
  EXPECT_NE(some_ref(y), Option(Some(101'101)));
}

TEST(OptionTest, Contains) {
  EXPECT_TRUE(Option(Some(vector{1, 2, 3, 4})).contains(vector{1, 2, 3, 4}));
  EXPECT_FALSE(
      Option(Some(vector{1, 2, 3, 4})).contains(vector{1, 2, 3, 4, 5}));

  EXPECT_TRUE(Option(Some(8)).contains(8));
  EXPECT_FALSE(Option(Some(8)).contains(88));
}

TEST(OptionLifetimeTest, Contains) {
  EXPECT_NO_THROW(Option(Some(make_mv<0>())).contains(make_mv<0>()));
  EXPECT_NO_THROW(Option<MoveOnly<1>>(None).contains(make_mv<1>()));
}

TEST(OptionTest, Exists) {
  constexpr auto even = [](int const& x) { return x % 2 == 0; };

  auto const all_even = [](vector<int> const& x) {
    return std::all_of(x.begin(), x.end(),
                       [](int const& x) { return x % 2 == 0; });
  };

  EXPECT_TRUE(make_some(8).exists(even));
  EXPECT_FALSE(make_some(81).exists(even));

  EXPECT_TRUE(make_some(vector{2, 4, 6, 8, 10}).exists(all_even));
  EXPECT_FALSE(make_some(vector{2, 4, 6, 9, 10}).exists(all_even));
}

TEST(OptionTest, AsConstRef) {
  Option const a = Some(68);
  EXPECT_EQ(a.as_cref().unwrap().get(), 68);

  Option<int> const b = None;
  EXPECT_EQ(b.as_cref(), None);

  Option const c = Some(vector{1, 2, 3, 4});
  EXPECT_EQ(c.as_cref().unwrap().get(), (vector{1, 2, 3, 4}));

  Option<vector<int>> const d = None;
  EXPECT_EQ(d.as_cref(), None);
}

TEST(OptionTest, AsRef) {
  Option a = Some(68);
  a.as_ref().unwrap().get() = 99;
  EXPECT_EQ(a, Some(99));

  Option<int> b = None;
  EXPECT_EQ(b.as_ref(), None);

  auto c = Option(Some(vector{1, 2, 3, 4}));
  c.as_ref().unwrap().get() = vector{5, 6, 7, 8, 9, 10};
  EXPECT_EQ(c, Some(vector{5, 6, 7, 8, 9, 10}));

  auto d = Option<vector<int>>(None);
  EXPECT_EQ(d.as_ref(), None);
}

TEST(OptionLifeTimeTest, AsRef) {
  auto a = Option(Some(make_mv<0>()));
  EXPECT_NO_THROW(a.as_ref().unwrap().get().done());

  auto b = Option<MoveOnly<1>>(None);
  EXPECT_NO_THROW(auto b_ = b.as_ref());
}

TEST(OptionTest, Unwrap) {
  EXPECT_EQ(Option(Some(0)).unwrap(), 0);
  EXPECT_DEATH_IF_SUPPORTED(Option<int>(None).unwrap(), ".*");

  EXPECT_EQ(Option(Some(vector{1, 2, 3, 4, 5})).unwrap(),
            (vector{1, 2, 3, 4, 5}));
  EXPECT_DEATH_IF_SUPPORTED(Option<vector<int>>(None).unwrap(), ".*");
}

TEST(OptionLifetimeTest, Unwrap) {
  auto a = Option(Some(make_mv<0>()));
  EXPECT_NO_THROW(move(a).unwrap().done());
}

TEST(OptionTest, Expect) {
  EXPECT_NO_THROW(Option(Some(0)).expect("No Value Received"));
  // how does it behave with unique_ptr?
  EXPECT_DEATH_IF_SUPPORTED(
      Option<unique_ptr<int>>(None).expect("No Value Received"), ".*");
}

TEST(OptionLifetimeTest, Expect) {
  auto a = Option(Some(make_mv<0>()));
  EXPECT_NO_THROW(move(a).expect("Yahoooo!").done());
}

TEST(OptionTest, UnwrapOr) {
  EXPECT_EQ(Option(Some(0)).unwrap_or(90), 0);
  EXPECT_EQ(Option<int>(None).unwrap_or(90), 90);

  EXPECT_EQ(
      Option(Some(vector{1, 2, 3, 4, 5})).unwrap_or(vector{6, 7, 8, 9, 10}),
      (vector{1, 2, 3, 4, 5}));
  EXPECT_EQ(Option<vector<int>>(None).unwrap_or(vector{6, 7, 8, 9, 10}),
            (vector{6, 7, 8, 9, 10}));
}

TEST(OptionLifetimeTest, UnwrapOr) {
  auto a = Option(Some(make_mv<0>()));
  EXPECT_NO_THROW(move(a).unwrap_or(make_mv<0>()).done());

  auto b = Option<MoveOnly<1>>(None);
  EXPECT_NO_THROW(move(b).unwrap_or(make_mv<1>()).done());
}

TEST(OptionTest, UnwrapOrElse) {
  auto&& a = Option(Some(0)).unwrap_or_else([]() { return 90; });
  EXPECT_EQ(a, 0);
  auto&& b = Option<int>(None).unwrap_or_else([]() { return 90; });
  EXPECT_EQ(b, 90);

  auto&& c = Option(Some(vector{1, 2, 3, 4, 5})).unwrap_or_else([]() {
    return vector{6, 7, 8, 9, 10};
  });
  EXPECT_EQ(c, (vector{1, 2, 3, 4, 5}));

  auto&& d = Option<vector<int>>(None).unwrap_or_else([]() {
    return vector{6, 7, 8, 9, 10};
  });
  EXPECT_EQ(d, (vector{6, 7, 8, 9, 10}));
}

TEST(OptionLifetimeTest, UnwrapOrElse) {
  auto a = Option(Some(make_mv<0>()));
  auto fn = []() { return make_mv<0>(); };
  EXPECT_NO_THROW(move(a).unwrap_or_else(fn).done());

  auto b = Option<MoveOnly<1>>(None);
  auto fn_b = []() { return make_mv<1>(); };
  EXPECT_NO_THROW(move(b).unwrap_or_else(fn_b).done());
}

TEST(OptionTest, Map) {
  auto&& a = Option(Some(90)).map([](int&& x) -> int { return x + 90; });
  EXPECT_EQ(a, Some(180));

  auto&& b = Option<int>(None).map([](int&& x) { return x + 90; });
  EXPECT_EQ(b, None);

  auto&& c = Option(Some(vector{1, 2, 3, 4, 5})).map([](vector<int>&& vec) {
    vec.push_back(6);
    return move(vec);
  });
  EXPECT_EQ(c, Some(vector{1, 2, 3, 4, 5, 6}));

  auto&& d = Option<vector<int>>(None).map([](vector<int>&& vec) {
    vec.push_back(6);
    return move(vec);
  });
  EXPECT_EQ(d, None);
}

TEST(OptionLifetimeTest, Map) {
  auto a = Option(Some(make_mv<0>()));
  EXPECT_NO_THROW(move(a).map([](auto r) { return r; }).unwrap().done());
}

TEST(OptionTest, FnMutMap) {
  auto fnmut_a = FnMut();
  auto a1_ = Option(Some(90)).map(fnmut_a);
  auto a2_ = Option(Some(90)).map(fnmut_a);

  EXPECT_EQ(fnmut_a.call_times, 2);

  auto const fnmut_b = FnMut();
  auto b1_ = Option(Some(90)).map(fnmut_b);
  auto b2_ = Option(Some(90)).map(fnmut_b);
  EXPECT_EQ(fnmut_b.call_times, 0);

  auto fnconst = FnConst();
  auto c = Option(Some(90)).map(fnconst);
}

TEST(OptionTest, MapOrElse) {
  auto&& a = Option(Some(90)).map_or_else([](int&& x) -> int { return x + 90; },
                                          []() -> int { return 90; });
  EXPECT_EQ(a, 180);

  auto&& b = Option<int>(None).map_or_else(
      [](int&& x) -> int { return x + 90; }, []() -> int { return 90; });
  EXPECT_EQ(b, 90);
}

TEST(OptionLifetimeTest, MapOrElse) {
  auto a = Option(Some(make_mv<0>()));
  auto fn = [](auto) { return make_mv<0>(); };  // NOLINT
  auto fn_b = []() { return make_mv<0>(); };    // NOLINT
  EXPECT_NO_THROW(move(a).map_or_else(fn, fn_b).done());
}

/*
TEST(OptionTest, OkOr) {
  enum class TestError { Good, Bad };
  EXPECT_EQ(Option(Some(90)).ok_or(TestError::Bad).unwrap(), 90);
  EXPECT_EQ(Option<int>(None).ok_or(TestError::Bad).unwrap_err(),
            TestError::Bad);

  EXPECT_EQ(Option(Some(vector{1, 2, 3, 4, 5})).ok_or(TestError::Bad).unwrap(),
            (vector{1, 2, 3, 4, 5}));
  EXPECT_EQ(Option<vector<int>>(None).ok_or(TestError::Bad).unwrap_err(),
            TestError::Bad);

  EXPECT_EQ(Option(Some(90)).ok_or(vector{-1, -2, -3, -4, -5}).unwrap(), 90);
  EXPECT_EQ(Option<int>(None).ok_or(vector{-1, -2, -3, -4, -5}).unwrap_err(),
            (vector{-1, -2, -3, -4, -5}));

  EXPECT_EQ(Option(Some(vector{1, 2, 3, 4, 5}))
                .ok_or(vector{-1, -2, -3, -4, -5})
                .unwrap(),
            (vector{1, 2, 3, 4, 5}));
  EXPECT_EQ(
      Option<vector<int>>(None).ok_or(vector{-1, -2, -3, -4, -5}).unwrap_err(),
      (vector{-1, -2, -3, -4, -5}));
}

TEST(OptionLifetimeTest, OkOr) {
  auto a = Option(Some(make_mv<0>()));
  EXPECT_NO_THROW(move(a).ok_or(make_mv<1>()).unwrap().done());
}

TEST(OptionTest, OkOrElse) {
  enum class TestError { Good, Bad };
  auto fn = []() { return TestError::Bad; };
  EXPECT_EQ(Option(Some(90)).ok_or_else(fn).unwrap(), 90);
  EXPECT_EQ(make_none<int>().ok_or_else(fn).unwrap_err(), TestError::Bad);

  EXPECT_EQ(Option(Some(vector{1, 2, 3, 4, 5})).ok_or_else(fn).unwrap(),
            (vector{1, 2, 3, 4, 5}));
  EXPECT_EQ(Option<vector<int>>(None).ok_or_else(fn).unwrap_err(),
            TestError::Bad);

  auto fnv = []() { return vector{-1, -2, -3, -4, -5}; };  // NOLINT
  EXPECT_EQ(Option(Some(90)).ok_or_else(fnv).unwrap(), 90);
  EXPECT_EQ(make_none<int>().ok_or_else(fnv).unwrap_err(),
            (vector{-1, -2, -3, -4, -5}));

  EXPECT_EQ(Option(Some(vector{1, 2, 3, 4, 5})).ok_or_else(fnv).unwrap(),
            (vector{1, 2, 3, 4, 5}));
  EXPECT_EQ(Option<vector<int>>(None).ok_or_else(fnv).unwrap_err(),
            (vector{-1, -2, -3, -4, -5}));
}

TEST(OptionLifetimeTest, OkOrElse) {
  enum class Err { OOM };
  auto a = Option(Some(make_mv<0>()));
  auto fn = []() { return make_mv<2>(); };
  EXPECT_NO_THROW(move(a).ok_or_else(fn).unwrap().done());
}
*/

TEST(OptionTest, And) {
  auto&& a = Option(Some(90)).AND(Option(Some(90.0f)));

  EXPECT_FLOAT_EQ(move(a).unwrap(), 90.0f);

  auto&& b = make_none<int>().AND(Option(Some(90.0f)));
  EXPECT_EQ(b, None);

  auto c = []() {
    return Option(Some(90)).AND(Option(Some(vector{90.0f, 180.0f, 3.141f})));
  };

  EXPECT_EQ(c().unwrap(), (vector{90.0f, 180.0f, 3.141f}));

  auto&& d = make_none<int>().AND(Option(Some(vector{90.0f, 180.0f, 3.141f})));
  EXPECT_EQ(d, None);
}

TEST(OptionLifetimeTest, And) {
  EXPECT_NO_THROW(Option(Some(make_mv<0>()))
                      .AND(Option(Some(make_mv<1>())))
                      .unwrap()
                      .done());
  EXPECT_EQ(make_none<int>().AND(Option(Some(make_mv<2>()))), None);
}

TEST(OptionTest, AndThen) {
  auto&& a = Option(Some(90)).and_then(
      [](int&& x) { return Option(Some(static_cast<float>(x) + 90.0f)); });

  EXPECT_FLOAT_EQ(move(a).unwrap(), 180.0f);

  auto&& b = make_none<int>().and_then(
      [](int&& x) { return Option(Some(static_cast<float>(x) + 90.0f)); });
  EXPECT_EQ(b, None);

  //
}

TEST(OptionTest, Filter) {
  auto is_even = [](int const& num) { return num % 2 == 0; };
  auto is_odd = [&](int const& num) { return !(is_even(num)); };

  EXPECT_EQ(Option(Some(90)).filter(is_even).unwrap(), 90);
  EXPECT_EQ(Option(Some(99)).filter(is_odd).unwrap(), 99);

  EXPECT_EQ(make_none<int>().filter(is_even), None);
  EXPECT_EQ(make_none<int>().filter(is_even), None);

  auto all_odd = [&](vector<int> const& vec) {
    return all_of(vec.begin(), vec.end(), is_odd);
  };

  EXPECT_EQ(Option(Some(vector{1, 3, 5, 7, 2, 4, 6, 8})).filter(all_odd), None);
  EXPECT_EQ(Option(Some(vector{1, 3, 5, 7})).filter(all_odd).unwrap(),
            (vector{1, 3, 5, 7}));

  EXPECT_EQ(Option<vector<int>>(None).filter(all_odd), None);
  EXPECT_EQ(Option<vector<int>>(None).filter(all_odd), None);
}

/*
TEST(OptionTest, FilterNot) {
  auto is_even = [](int const& num) { return num % 2 == 0; };

  EXPECT_EQ(Option(Some(90)).filter_not(is_even), None);
  EXPECT_EQ(Option(Some(99)).filter_not(is_even), Some(99));

  EXPECT_EQ(make_none<int>().filter_not(is_even), None);
  EXPECT_EQ(make_none<int>().filter_not(is_even), None);

  auto all_odd = [&](vector<int> const& vec) {
    return all_of(vec.begin(), vec.end(), [=](auto x) { return !is_even(x); });
  };

  EXPECT_EQ(Option(Some(vector{1, 3, 5, 7, 2, 4, 6, 8})).filter_not(all_odd),
            Some(vector{1, 3, 5, 7, 2, 4, 6, 8}));
  EXPECT_EQ(make_some(vector{1, 3, 5, 7}).filter_not(all_odd), None);

  EXPECT_EQ(make_none<vector<int>>().filter_not(all_odd), None);
  EXPECT_EQ(make_none<vector<int>>().filter_not(all_odd), None);
}
*/

TEST(OptionTest, Or) {
  auto&& a = Option(Some(90)).OR(Option(Some(89)));
  EXPECT_EQ(move(a).unwrap(), 90);

  auto&& b = make_none<int>().OR(Option(Some(89)));
  EXPECT_EQ(move(b).unwrap(), 89);

  auto&& c = make_none<int>().OR(make_none<int>());
  EXPECT_EQ(c, None);
  //
  //
  auto&& d = Option(Some(vector{1, 2, 3, 4, 5}))
                 .OR(Option(Some(vector{6, 7, 8, 9, 10})));
  EXPECT_EQ(move(d).unwrap(), (vector{1, 2, 3, 4, 5}));

  auto&& e = Option<vector<int>>(None).OR(Option(Some(vector{6, 7, 8, 9, 10})));
  EXPECT_EQ(move(e).unwrap(), (vector{6, 7, 8, 9, 10}));

  auto&& f = Option<vector<int>>(None).OR(Option<vector<int>>(None));
  EXPECT_EQ(f, None);
}

/*
TEST(OptionTest, Xor) {
  auto&& a = Option(Some(90)).XOR(Option(Some(89)));
  EXPECT_EQ(a, None);

  auto&& b = make_none<int>().XOR(Option(Some(89)));
  EXPECT_EQ(move(b).unwrap(), 89);

  auto&& c = make_none<int>().XOR(make_none<int>());
  EXPECT_EQ(c, None);
  //
  //
  auto&& d = Option(Some(vector{1, 2, 3, 4, 5}))
                 .XOR(Option(Some(vector{6, 7, 8, 9, 10})));
  EXPECT_EQ(d, None);

  auto&& e =
      Option<vector<int>>(None).XOR(Option(Some(vector{6, 7, 8, 9, 10})));
  EXPECT_EQ(move(e).unwrap(), (vector{6, 7, 8, 9, 10}));

  auto&& f = Option<vector<int>>(None).XOR(Option<vector<int>>(None));
  EXPECT_EQ(f, None);
}
*/

TEST(OptionTest, Take) {
  auto a = Option(Some(9));
  EXPECT_EQ(a.take().unwrap(), 9);
  EXPECT_EQ(a, None);

  auto b = Option<int>(None);
  EXPECT_EQ(b.take(), None);
  EXPECT_EQ(b, None);

  auto c = Option(Some(vector{-1, -2, -4, -8, -16}));
  auto ca = c.take();
  EXPECT_EQ(ca, Some(vector{-1, -2, -4, -8, -16}));
  EXPECT_EQ(c, None);

  auto d = Option<vector<int>>(None);
  EXPECT_EQ(d.take(), None);
  EXPECT_EQ(d, None);
}

TEST(OptionTest, Replace) {
  auto a = Option(Some(9));
  EXPECT_EQ(a.replace(27), Some(9));
  EXPECT_EQ(a, Some(27));

  auto b = Option<int>(None);
  EXPECT_EQ(b.replace(88), None);
  EXPECT_EQ(b, Some(88));

  auto c = Option(Some(vector{-1, -2, -4, -8, -16}));
  EXPECT_EQ(c.replace(vector<int>{}), Some(vector{-1, -2, -4, -8, -16}));
  EXPECT_EQ(c, Some(vector<int>{}));

  auto d = Option<vector<int>>(None);
  EXPECT_EQ(d.replace(vector<int>{1, 2, 3, 4, 5}), None);
  EXPECT_EQ(d, Some(vector<int>{1, 2, 3, 4, 5}));
}

TEST(OptionTest, Copy) {
  auto a = Option(Some(9));
  EXPECT_EQ(a.copy(), Some(9));
  EXPECT_EQ(a, Some(9));

  auto b = Option(Some<int*>(nullptr));
  EXPECT_EQ(b.copy(), Some<int*>(nullptr));
  EXPECT_EQ(b, Some<int*>(nullptr));

  auto c = Option(Some(vector<int>{1, 2, 3, 4, 5}));
  EXPECT_EQ(c.copy(), Some(vector<int>{1, 2, 3, 4, 5}));
  EXPECT_EQ(c, Some(vector<int>{1, 2, 3, 4, 5}));
}

TEST(OptionTest, OrElse) {
  auto&& a = Option(Some(90.0f)).or_else([]() { return Option(Some(0.5f)); });
  EXPECT_FLOAT_EQ(move(a).unwrap(), 90.0f);

  auto&& b = Option<float>(None).or_else([]() { return Option(Some(0.5f)); });
  EXPECT_FLOAT_EQ(move(b).unwrap(), 0.5f);

  auto&& c = Option<float>(None).or_else([]() { return Option<float>(None); });
  EXPECT_EQ(c, None);
  //
  //
  auto&& d = Option(Some(vector{1, 2, 3, 4, 5})).or_else([]() {
    return Option(Some(vector{6, 7, 8, 9, 10}));
  });
  EXPECT_EQ(move(d).unwrap(), (vector{1, 2, 3, 4, 5}));

  auto&& e = Option<vector<int>>(None).or_else([]() {
    return Option(Some(vector{6, 7, 8, 9, 10}));
  });
  EXPECT_EQ(move(e).unwrap(), (vector{6, 7, 8, 9, 10}));

  auto&& f = Option<vector<int>>(None).or_else(
      []() { return Option<vector<int>>(None); });
  EXPECT_EQ(f, None);
}

TEST(OptionTest, ExpectNone) {
  EXPECT_DEATH_IF_SUPPORTED(Option(Some(56)).expect_none("===TEST==="), ".*");
  EXPECT_NO_THROW(Option<int>(None).expect_none("===TEST==="));

  EXPECT_DEATH_IF_SUPPORTED(
      Option(Some(vector<int>{1, 2, 3, 4, 5})).expect_none("===TEST==="), ".*");
  EXPECT_NO_THROW(Option<vector<int>>(None).expect_none("===TEST==="));
}

TEST(OptionTest, UnwrapNone) {
  EXPECT_DEATH_IF_SUPPORTED(Option(Some(56)).unwrap_none(), ".*");
  EXPECT_NO_THROW(Option<int>(None).unwrap_none());

  EXPECT_DEATH_IF_SUPPORTED(
      Option(Some(vector<int>{1, 2, 3, 4, 5})).unwrap_none(), ".*");
  EXPECT_NO_THROW(Option<vector<int>>(None).unwrap_none());
}

TEST(OptionTest, UnwrapOrDefault) {
  EXPECT_EQ(Option(Some(0)).unwrap_or_default(), 0);
  EXPECT_EQ(Option<int>(None).unwrap_or_default(), 0);

  EXPECT_EQ(Option(Some(vector{1, 2, 3, 4, 5})).unwrap_or_default(),
            (vector{1, 2, 3, 4, 5}));
  EXPECT_EQ(Option<vector<int>>(None).unwrap_or_default(), (vector<int>{}));
}

TEST(OptionTest, Match) {
  auto v = Option(Some(98)).match([](auto some) { return some + 2; },
                                  []() { return 5; });
  EXPECT_EQ(v, 100);

  auto&& a = Option(Some(90)).match([](int&& x) { return x + 10; },
                                    []() { return -1; });
  EXPECT_EQ(a, 100);

  auto&& b = Option<int>(None).match([](auto&& x) { return x + 10; },
                                     []() { return -1; });
  EXPECT_EQ(b, -1);

  auto&& c =
      Option(Some(vector{1, 2, 3, 4, 5}))
          .match([](auto&& x) { return accumulate(x.begin(), x.end(), 0); },
                 []() { return -1; });
  EXPECT_EQ(c, 15);

  auto&& d = Option<vector<int>>(None).match(
      [](auto&& x) { return accumulate(x.begin(), x.end(), 0); },
      []() { return -1; });
  EXPECT_EQ(d, -1);
}

auto opt_try_b(int x) -> Option<int> {
  if (x > 0) {
    return Some(std::move(x));
  } else {
    return None;
  }
}

#include "stx/try_some.h"

auto opt_try_a(int m) -> Option<int> {
  // clang-format off
  TRY_SOME(x, opt_try_b(m)); TRY_SOME(const y, opt_try_b(m)); TRY_SOME(volatile z, opt_try_b(m));
  auto opt = opt_try_b(m);
  TRY_SOME(w, std::move(opt));
  // clang-format on
  x += 60;
  return Some(std::move(x));
}

TEST(OptionTest, TrySome) {
  EXPECT_EQ(opt_try_a(10), Some(70));
  EXPECT_EQ(opt_try_a(100'000), Some(100'060));
  EXPECT_EQ(opt_try_a(-1), None);
  EXPECT_EQ(opt_try_a(-10), None);
}

TEST(OptionTest, Docs) {}
