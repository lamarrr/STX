/**
 * @file result_test.cc
 * @author Basit Ayantunde <rlamarrr@gmail.com>
 * @brief
 * @version  0.1
 * @date 2020-04-16
 *
 * @copyright Copyright (c) 2020
 *
 */

#include <numeric>
//
#include "fmt/format.h"
#include "gtest/gtest.h"
#include "stx/result.h"

using namespace std;
using namespace string_literals;
using namespace stx;

template <typename T, typename E>
inline Result<T, E> MakeOk(T&& t) {
  return Result<T, E>(Ok<T>(forward<T>(t)));
}
template <typename T, typename E>
inline Result<T, E> MakeErr(E&& e) {
  return Result<T, E>(Err<E>(forward<E>(e)));
}

TEST(ResultTest, Equality) {
  //
  EXPECT_EQ((MakeOk<int, int>(78)), Ok(78));
  EXPECT_NE((MakeOk<int, int>(7)), Ok(78));
  EXPECT_NE((MakeOk<int, int>(78)), Err(78));

  EXPECT_NE((MakeErr<int, int>(78)), Ok(78));
  EXPECT_NE((MakeErr<int, int>(7)), Ok(78));
  EXPECT_NE((MakeErr<int, int>(78)), Err(-78));
  EXPECT_EQ((MakeErr<int, int>(78)), Err(78));

  EXPECT_EQ((MakeOk<vector<int>, int>(vector{1, 2, 3, 4, 5})),
            Ok(vector{1, 2, 3, 4, 5}));
  EXPECT_EQ((MakeOk<vector<int>, int>(vector{1, 2, 3, 4, 5})),
            (MakeOk<vector<int>, int>(vector{1, 2, 3, 4, 5})));

  EXPECT_NE((MakeOk<vector<int>, vector<int>>(vector{1, 2, 3, 4, 5})),
            Err(vector{1, 2, 3, 4, 5}));
  EXPECT_NE((MakeOk<vector<int>, vector<int>>(vector{1, 2, 3, 4, 5})),
            (MakeErr<vector<int>, vector<int>>(vector{1, 2, 3, 4, 5})));

  int const f = 89;
  int g = 89;

  EXPECT_EQ(Ok(89), Ok<ConstRef<int>>(f));
  EXPECT_EQ(Ok(89), Ok<MutRef<int>>(g));

  EXPECT_NE(Ok(8), Ok<ConstRef<int>>(f));
  EXPECT_NE(Ok(8), Ok<MutRef<int>>(g));

  EXPECT_EQ(Ok(89), Ok(&g));
  EXPECT_EQ(Ok(89), Ok(&f));

  EXPECT_NE(Ok(9), Ok(&g));
  EXPECT_NE(Ok(9), Ok(&f));

  int const h = 89;
  int i = 89;

  EXPECT_EQ(Err(89), Err<ConstRef<int>>(h));
  EXPECT_EQ(Err(89), Err<MutRef<int>>(i));

  EXPECT_NE(Err(8), Err<ConstRef<int>>(h));
  EXPECT_NE(Err(8), Err<MutRef<int>>(i));

  EXPECT_EQ(Err(89), Err(&h));
  EXPECT_EQ(Err(89), Err(&i));

  EXPECT_NE(Err(8), Err(&h));
  EXPECT_NE(Err(8), Err(&i));

  EXPECT_EQ((MakeOk<int, int>(89)), Ok(&h));
  EXPECT_EQ((MakeOk<int, int>(89)), Ok(&i));

  EXPECT_EQ((MakeOk<int, int>(89)), Ok<ConstRef<int>>(h));
  EXPECT_EQ((MakeOk<int, int>(89)), Ok<ConstRef<int>>(i));

  EXPECT_EQ((MakeOk<int, int>(89)), Ok<MutRef<int>>(i));

  EXPECT_EQ((MakeErr<int, int>(89)), Err(&h));
  EXPECT_EQ((MakeErr<int, int>(89)), Err(&i));

  EXPECT_EQ((MakeErr<int, int>(89)), Err<ConstRef<int>>(h));
  EXPECT_EQ((MakeErr<int, int>(89)), Err<ConstRef<int>>(i));

  EXPECT_EQ((MakeErr<int, int>(89)), Err<MutRef<int>>(i));
}

TEST(ResultTest, IsOk) {
  EXPECT_TRUE((MakeOk<int, int>(0).is_ok()));
  EXPECT_FALSE((MakeErr<int, int>(9).is_ok()));

  EXPECT_TRUE((MakeOk<vector<int>, int>(vector{1, 2, 3, 4}).is_ok()));
  EXPECT_FALSE((MakeErr<vector<int>, int>(89).is_ok()));

  EXPECT_TRUE((MakeOk<int, vector<int>>(-78).is_ok()));
  EXPECT_FALSE((MakeErr<int, vector<int>>(vector{1, 2, 3, 4}).is_ok()));

  EXPECT_TRUE((MakeOk<vector<int>, vector<int>>(vector{1, 2, 3, 4}).is_ok()));
  EXPECT_FALSE((MakeErr<vector<int>, vector<int>>(vector{1, 2, 3, 4}).is_ok()));
}

TEST(ResultTest, IsErr) {
  EXPECT_TRUE((MakeErr<int, int>(9).is_err()));
  EXPECT_FALSE((MakeOk<int, int>(0).is_err()));

  EXPECT_TRUE((MakeErr<vector<int>, int>(89).is_err()));
  EXPECT_FALSE((MakeOk<vector<int>, int>(vector{1, 2, 3, 4}).is_err()));

  EXPECT_TRUE((MakeErr<int, vector<int>>(vector{1, 2, 3, 4}).is_err()));
  EXPECT_FALSE((MakeOk<int, vector<int>>(99).is_err()));

  EXPECT_TRUE((MakeErr<vector<int>, vector<int>>(vector{5, 6, 7, 8}).is_err()));
  EXPECT_FALSE((MakeOk<vector<int>, vector<int>>(vector{1, 2, 3, 4}).is_err()));
}

TEST(ResultTest, Contains) {
  EXPECT_TRUE((MakeOk<int, int>(9).contains(9)));
  EXPECT_FALSE((MakeOk<int, int>(10).contains(0)));
  EXPECT_FALSE((MakeErr<int, int>(0).contains(0)));

  EXPECT_TRUE(
      (MakeOk<vector<int>, int>({1, 2, 3, 4}).contains(vector{1, 2, 3, 4})));
  EXPECT_FALSE(
      (MakeOk<vector<int>, int>({1, 2, 3, 4}).contains(vector{5, 6, 7, 8})));
  EXPECT_FALSE((MakeErr<vector<int>, int>(89).contains(vector{1, 2, 3, 4})));

  EXPECT_TRUE((MakeOk<int, vector<int>>(9).contains(9)));
  EXPECT_FALSE((MakeOk<int, vector<int>>(10).contains(0)));
  EXPECT_FALSE((MakeErr<int, vector<int>>(vector{4, 5, 6, 7, 8}).contains(0)));

  EXPECT_TRUE((MakeOk<vector<int>, vector<int>>(vector{1, 2, 3, 4})
                   .contains(vector{1, 2, 3, 4})));
  EXPECT_FALSE((MakeOk<vector<int>, vector<int>>(vector{1, 2, 3, 4})
                    .contains(vector{5, 6, 7, 8})));
  EXPECT_FALSE((MakeErr<vector<int>, vector<int>>(vector{5, 6, 7, 8, 9})
                    .contains(vector{1, 2, 3, 4})));
}

TEST(ResultTest, ContainsErr) {
  EXPECT_TRUE((MakeErr<int, int>(0).contains_err(0)));
  EXPECT_FALSE((MakeErr<int, int>(9).contains_err(20)));
  EXPECT_FALSE((MakeOk<int, int>(10).contains_err(10)));

  EXPECT_TRUE((MakeErr<vector<int>, int>(0).contains_err(0)));
  EXPECT_FALSE((MakeErr<vector<int>, int>(9).contains_err(20)));
  EXPECT_FALSE((MakeOk<vector<int>, int>(vector{1, 2, 3, 4}).contains_err(10)));

  EXPECT_TRUE((MakeErr<int, vector<int>>(vector{1, 2, 3, 4})
                   .contains_err(vector{1, 2, 3, 4})));
  EXPECT_FALSE((MakeErr<int, vector<int>>(vector{1, 2, 3, 4})
                    .contains_err(vector{5, 6, 7, 8})));
  EXPECT_FALSE((MakeOk<int, vector<int>>(89).contains_err(vector{1, 2, 3, 4})));

  EXPECT_TRUE((MakeErr<vector<int>, vector<int>>(vector{1, 2, 3, 4})
                   .contains_err(vector{1, 2, 3, 4})));
  EXPECT_FALSE((MakeErr<vector<int>, vector<int>>(vector{1, 2, 3, 4})
                    .contains_err(vector{5, 6, 7, 8})));
  EXPECT_FALSE((MakeOk<vector<int>, vector<int>>(vector{1, 2, 3, 4})
                    .contains_err(vector{1, 2, 3, 4})));
}

TEST(ResultTest, Ok) {
  EXPECT_EQ((MakeOk<int, int>(20).ok().unwrap()), 20);
  EXPECT_ANY_THROW((MakeErr<int, int>(90).ok().unwrap()));

  EXPECT_EQ((MakeOk<vector<int>, int>(vector{1, 2, 3, 4}).ok().unwrap()),
            (vector{1, 2, 3, 4}));
  EXPECT_ANY_THROW((MakeErr<vector<int>, int>(90).ok().unwrap()));

  EXPECT_EQ((MakeOk<int, vector<int>>(90).ok().unwrap()), 90);
  EXPECT_ANY_THROW(
      (MakeErr<int, vector<int>>(vector{1, 2, 3, 4}).ok().unwrap()));

  EXPECT_EQ(
      (MakeOk<vector<int>, vector<int>>(vector{1, 2, 3, 4}).ok().unwrap()),
      (vector{1, 2, 3, 4}));
  EXPECT_ANY_THROW(
      (MakeErr<vector<int>, vector<int>>(vector{1, 2, 3, 4}).ok().unwrap()));
}

TEST(ResultTest, Err) {
  EXPECT_EQ((MakeErr<int, int>(20).err().unwrap()), 20);
  EXPECT_ANY_THROW((MakeOk<int, int>(90).err().unwrap()));

  EXPECT_EQ((MakeErr<vector<int>, int>(10).err().unwrap()), 10);
  EXPECT_ANY_THROW(
      (MakeOk<vector<int>, int>(vector{1, 2, 3, 4}).err().unwrap()));

  EXPECT_EQ((MakeErr<int, vector<int>>(vector{1, 2, 3, 4}).err().unwrap()),
            (vector{1, 2, 3, 4}));
  EXPECT_ANY_THROW((MakeOk<int, vector<int>>(78).err().unwrap()));

  EXPECT_EQ(
      (MakeErr<vector<int>, vector<int>>(vector{1, 2, 3, 4}).err().unwrap()),
      (vector{1, 2, 3, 4}));
  EXPECT_ANY_THROW(
      (MakeOk<vector<int>, vector<int>>(vector{78, 67}).err().unwrap()));
}

TEST(ResultTest, AsConstRef) {
  auto const a = MakeOk<int, int>(68);
  EXPECT_EQ(a.as_const_ref().unwrap().get(), 68);
  auto const b = MakeErr<int, int>(-68);
  EXPECT_ANY_THROW(b.as_const_ref().unwrap());

  auto const c = MakeOk<vector<int>, int>(vector{1, 2, 3, 4});
  EXPECT_EQ(c.as_const_ref().unwrap().get(), (vector{1, 2, 3, 4}));
  auto const d = MakeErr<vector<int>, int>(-24);
  EXPECT_ANY_THROW(d.as_const_ref().unwrap());

  auto const e = MakeOk<int, vector<int>>(-24);
  EXPECT_EQ(e.as_const_ref().unwrap().get(), -24);
  auto const f = MakeErr<int, vector<int>>(vector{-1, -2, -3, -4});
  EXPECT_ANY_THROW(f.as_const_ref().unwrap().get());

  auto const g = MakeOk<vector<int>, vector<int>>(vector{1, 2, 3, 4});
  EXPECT_EQ(g.as_const_ref().unwrap().get(), (vector{1, 2, 3, 4}));
  auto const h = MakeErr<vector<int>, vector<int>>(vector{-1, -2, -3, -4});
  EXPECT_ANY_THROW(h.as_const_ref().unwrap().get());
}

TEST(ResultTest, AsMutRef) {
  auto a = MakeOk<int, int>(68);
  a.as_mut_ref().unwrap().get() = 89;
  EXPECT_EQ(a.as_mut_ref().unwrap().get(), 89);

  auto b = MakeErr<int, int>(-68);
  EXPECT_ANY_THROW(b.as_mut_ref().unwrap().get());
  EXPECT_EQ(b.as_mut_ref().unwrap_err().get(), -68);

  auto c = MakeOk<vector<int>, int>(vector{1, 2, 3, 4});
  c.as_mut_ref().unwrap().get() = vector{-1, -2, -3, -4};
  EXPECT_EQ(c.as_mut_ref().unwrap().get(), (vector{-1, -2, -3, -4}));

  auto d = MakeErr<vector<int>, int>(-24);
  EXPECT_ANY_THROW(d.as_mut_ref().unwrap().get());
  EXPECT_EQ(d.as_mut_ref().unwrap_err(), -24);

  auto e = MakeOk<int, vector<int>>(-24);
  e.as_mut_ref().unwrap().get() = 87;
  EXPECT_EQ(e.as_mut_ref().unwrap().get(), 87);

  auto f = MakeErr<int, vector<int>>(vector{-1, -2, -3, -4});
  EXPECT_ANY_THROW(f.as_mut_ref().unwrap().get());
  EXPECT_EQ(f.as_mut_ref().unwrap_err().get(), (vector{-1, -2, -3, -4}));

  auto g = MakeOk<vector<int>, vector<int>>(vector{1, 2, 3, 4});
  EXPECT_EQ(g.as_mut_ref().unwrap().get(), (vector{1, 2, 3, 4}));
  EXPECT_ANY_THROW(g.as_mut_ref().unwrap_err().get());

  auto h = MakeErr<vector<int>, vector<int>>(vector{-1, -2, -3, -4});
  EXPECT_ANY_THROW(h.as_mut_ref().unwrap().get());
  h.as_mut_ref().unwrap_err().get() = vector{-5, -6, -7, -8, -9};
  EXPECT_EQ(h.as_mut_ref().unwrap_err().get(), (vector{-5, -6, -7, -8, -9}));
}

TEST(ResultTest, Map) {
  // will it be problematic if the map function is not in scope? No!
  auto a = [](int&& value) { return value + 20; };
  EXPECT_EQ((MakeOk<int, int>(20).map(a).unwrap()), 40);
  EXPECT_ANY_THROW((MakeErr<int, int>(-1).map(a).unwrap()));

  // will it be problematic if the map function is not in scope? No!
  auto b = [](vector<int>&& value) {
    value.push_back(6);
    return move(value);
  };
  EXPECT_EQ((MakeOk<vector<int>, int>({1, 2, 3, 4, 5}).map(b).unwrap()),
            (vector{1, 2, 3, 4, 5, 6}));
  EXPECT_ANY_THROW((MakeErr<vector<int>, int>(-1).map(b).unwrap()));
}

TEST(ResultTest, MapOr) {
  auto a = [](int&& value) { return value + 20; };
  EXPECT_EQ((MakeOk<int, int>(20).map_or(a, 100)), 40);
  EXPECT_EQ((MakeErr<int, int>(-20).map_or(a, 100)), 100);

  auto b = [](vector<int>&& value) {
    value.push_back(6);
    return move(value);
  };
  EXPECT_EQ(
      (MakeOk<vector<int>, int>({1, 2, 3, 4, 5}).map_or(b, vector<int>{})),
      (vector<int>{1, 2, 3, 4, 5, 6}));
  EXPECT_EQ(
      (MakeErr<vector<int>, int>(-20).map_or(b, vector<int>{6, 7, 8, 9, 10})),
      (vector<int>{6, 7, 8, 9, 10}));
}

TEST(ResultTest, MapOrElse) {
  auto a = [](int&& value) { return value + 20; };
  auto else_a = [](auto) { return -10; };

  EXPECT_EQ((MakeOk<int, int>(20).map_or_else(a, else_a)), 40);
  EXPECT_EQ((MakeErr<int, int>(-20).map_or_else(a, else_a)), -10);

  auto b = [](vector<int>&& value) {
    value.push_back(6);
    return move(value);
  };
  auto else_b = [](auto) -> vector<int> { return {6, 7, 8, 9, 10}; };  // NOLINT

  EXPECT_EQ((MakeOk<vector<int>, int>({1, 2, 3, 4, 5}).map_or_else(b, else_b)),
            (vector<int>{1, 2, 3, 4, 5, 6}));
  EXPECT_EQ((MakeErr<vector<int>, int>(-20).map_or_else(b, else_b)),
            (vector<int>{6, 7, 8, 9, 10}));
}

TEST(ResultTest, MapErr) {
  // will it be problematic if the map function is not in scope? No!
  auto a = [](int&& value) { return value * 10; };
  EXPECT_EQ((MakeErr<int, int>(10).map_err(a).unwrap_err()), 100);
  EXPECT_ANY_THROW((MakeOk<int, int>(20).map_err(a).unwrap_err()));

  // will it be problematic if the map function is not in scope? No!
  auto b = [](vector<int>&& value) {
    value.push_back(6);
    return move(value);
  };
  EXPECT_EQ(
      (MakeErr<int, vector<int>>({1, 2, 3, 4, 5}).map_err(b).unwrap_err()),
      (vector{1, 2, 3, 4, 5, 6}));

  EXPECT_ANY_THROW((MakeOk<int, vector<int>>(256).map_err(b).unwrap_err()));
}

TEST(ResultTest, And) {
  EXPECT_FLOAT_EQ(
      (MakeOk<int, int>(20).AND(MakeOk<float, int>(40.0f)).unwrap()), 40.0f);

  EXPECT_ANY_THROW(
      (MakeErr<int, int>(-20).AND(MakeOk<float, int>(40.0f)).unwrap()));

  EXPECT_EQ((MakeOk<int, int>(20)
                 .AND(MakeOk<vector<float>, int>({40.0f, 50.0f, 60.0f}))
                 .unwrap()),
            (vector{40.0f, 50.0f, 60.0f}));

  EXPECT_ANY_THROW(
      (MakeErr<int, int>(-20)
           .AND(MakeOk<vector<float>, int>(vector{40.0f, 50.0f, 60.0f}))
           .unwrap()));

  EXPECT_EQ((MakeErr<int, int>(-20)
                 .AND(MakeOk<vector<float>, int>(vector{40.0f, 50.0f, 60.0f}))
                 .unwrap_err()),
            -20);
}

TEST(ResultTest, AndThen) {
  auto a = [](int v) { return v * 2.0f; };
  EXPECT_FLOAT_EQ((MakeOk<int, int>(20).and_then(a).unwrap()), 40.0f);

  EXPECT_ANY_THROW((MakeErr<int, int>(-20).and_then(a).unwrap()));

  EXPECT_EQ((MakeErr<int, int>(-20).and_then(a).unwrap_err()), -20);

  auto b = [](int&& v) {
    vector<float> res;
    res.push_back(static_cast<float>(v));
    return res;
  };

  EXPECT_EQ((MakeOk<int, int>(80).and_then(b).unwrap()), (vector{80.0f}));

  EXPECT_ANY_THROW((MakeErr<int, int>(-20).and_then(b).unwrap()));
  EXPECT_EQ((MakeErr<int, int>(-20).and_then(b).unwrap_err()), -20);
}

TEST(ResultTest, Or) {
  EXPECT_EQ((MakeOk<int, int>(20).OR(MakeOk<int, int>(40)).unwrap()), 20);

  EXPECT_EQ((MakeOk<int, int>(20).OR(MakeErr<int, int>(40)).unwrap()), 20);

  EXPECT_ANY_THROW((MakeErr<int, int>(-20).OR(MakeErr<int, int>(40)).unwrap()));

  EXPECT_EQ((MakeErr<int, int>(-20).OR(MakeErr<int, int>(40)).unwrap_err()),
            40);

  EXPECT_EQ((MakeOk<vector<int>, int>(vector{1, 2, 3, 4, 5})
                 .OR(MakeOk<vector<int>, int>(vector{40, 50, 60}))
                 .unwrap()),
            (vector{1, 2, 3, 4, 5}));

  EXPECT_EQ((MakeOk<vector<int>, int>(vector{1, 2, 3, 4, 5})
                 .OR(MakeErr<vector<int>, int>(8))
                 .unwrap()),
            (vector{1, 2, 3, 4, 5}));

  EXPECT_ANY_THROW((MakeErr<vector<int>, int>(-9)
                        .OR(MakeErr<vector<int>, int>(-8))
                        .unwrap()));

  EXPECT_EQ((MakeErr<vector<float>, int>(-20)
                 .OR(MakeOk<vector<float>, int>(vector{40.0f, 50.0f, 60.0f}))
                 .unwrap()),
            (vector{40.0f, 50.0f, 60.0f}));
}

TEST(ResultTest, OrElse) {
  auto a = [](int&& err) -> Result<int, int> { return Ok(err * 100); };
  EXPECT_EQ((MakeOk<int, int>(20).or_else(a).unwrap()), 20);
  EXPECT_EQ((MakeErr<int, int>(10).or_else(a).unwrap()), 1000);

  auto b = [](string&& err) -> Result<int, string> {
    return Err("Err: " + err);
  };
  EXPECT_EQ((MakeOk<int, string>(20).or_else(b).unwrap()), 20);
  EXPECT_EQ((MakeErr<int, string>("Max Limit"s).or_else(b).unwrap_err()),
            "Err: Max Limit"s);

  auto c = [](vector<int>&& err) -> Result<int, vector<int>> {
    return Ok(err.empty() ? -1 : err[0]);
  };
  EXPECT_EQ((MakeOk<int, vector<int>>(40).or_else(c).unwrap()), 40);
  EXPECT_EQ((MakeErr<int, vector<int>>(vector{10, 20, 30}).or_else(c).unwrap()),
            10);
}

TEST(ResultTest, UnwrapOr) {
  EXPECT_EQ((MakeOk<int, int>(89).unwrap_or(90)), 89);
  EXPECT_EQ((MakeErr<int, int>(89).unwrap_or(90)), 90);

  EXPECT_EQ((MakeOk<string, int>("John Doe"s).unwrap_or("Unknown"s)),
            "John Doe"s);
  EXPECT_EQ((MakeErr<string, int>(-20).unwrap_or("Unknown"s)), "Unknown"s);
}

TEST(ResultTest, Unwrap) {
  EXPECT_EQ((MakeOk<int, int>(89).unwrap()), 89);
  EXPECT_ANY_THROW((MakeErr<int, int>(89).unwrap()));

  EXPECT_EQ((MakeOk<string, int>("John Doe"s).unwrap()), "John Doe"s);
  EXPECT_ANY_THROW((MakeErr<string, int>(-20).unwrap()));

  EXPECT_EQ((MakeOk<vector<int>, int>(vector{1, 2, 3, 4, 5}).unwrap()),
            (vector{1, 2, 3, 4, 5}));
  EXPECT_ANY_THROW((MakeErr<vector<int>, int>(-1).unwrap()));
}

TEST(ResultTest, UnwrapOrElse) {
  auto a = [](int&& err) { return err + 20; };
  EXPECT_EQ((MakeOk<int, int>(10).unwrap_or_else(a)), 10);
  EXPECT_EQ((MakeErr<int, int>(20).unwrap_or_else(a)), 40);

  auto b = [](string&& err) -> int { return stoi(err) + 20; };
  EXPECT_EQ((MakeOk<int, string>(10).unwrap_or_else(b)), 10);
  EXPECT_EQ((MakeErr<int, string>("40"s).unwrap_or_else(b)), 60);

  auto c = [](vector<int>&& vec) {
    vec.push_back(10);
    return move(vec);
  };
  EXPECT_EQ((MakeOk<vector<int>, vector<int>>(vector{1, 2, 3, 4, 5})
                 .unwrap_or_else(c)),
            (vector{1, 2, 3, 4, 5}));
  EXPECT_EQ(
      (MakeErr<vector<int>, vector<int>>(vector{6, 7, 8, 9}).unwrap_or_else(c)),
      (vector{6, 7, 8, 9, 10}));
}

TEST(ResultTest, Expect) {
  EXPECT_EQ((MakeOk<int, int>(10).expect("===TEST ERR MSG===")), 10);
  EXPECT_ANY_THROW((MakeErr<int, int>(20).expect("===TEST ERR MSG===")));

  EXPECT_EQ((MakeOk<vector<int>, int>(vector{1, 2, 3, 4, 5})
                 .expect("===TEST ERR MSG===")),
            (vector{1, 2, 3, 4, 5}));
  EXPECT_ANY_THROW(
      (MakeErr<vector<int>, int>(20).expect("===TEST ERR MSG===")));

  EXPECT_EQ((MakeOk<int, vector<int>>(-1).expect("===TEST ERR MSG===")), -1);
  EXPECT_ANY_THROW((MakeErr<int, vector<int>>(vector{-1, -2, -3, -4, -5})
                        .expect("===TEST ERR MSG===")));

  EXPECT_EQ((MakeOk<vector<int>, vector<int>>(vector{1, 2, 3, 4, 5})
                 .expect("===TEST ERR MSG===")),
            (vector{1, 2, 3, 4, 5}));

  EXPECT_ANY_THROW(
      (MakeErr<vector<int>, vector<int>>(vector{-1, -2, -3, -4, -5})
           .expect("===TEST ERR MSG===")));
}

TEST(ResultTest, UnwrapErr) {
  EXPECT_EQ((MakeErr<int, int>(20).unwrap_err()), 20);
  EXPECT_ANY_THROW((MakeOk<int, int>(10).unwrap_err()));

  EXPECT_EQ((MakeErr<vector<int>, int>(-40).unwrap_err()), -40);
  EXPECT_ANY_THROW((MakeOk<vector<int>, int>(vector{10, 20, 30}).unwrap_err()));

  EXPECT_EQ((MakeErr<int, vector<int>>(vector{1, 2, 3, 4}).unwrap_err()),
            (vector{1, 2, 3, 4}));
  EXPECT_ANY_THROW((MakeOk<int, vector<int>>(68).unwrap_err()));

  EXPECT_EQ(
      (MakeErr<vector<int>, vector<int>>(vector{1, 2, 3, 4}).unwrap_err()),
      (vector{1, 2, 3, 4}));
  EXPECT_ANY_THROW(
      (MakeOk<vector<int>, vector<int>>(vector{1, 2, 3, 4}).unwrap_err()));
}

TEST(ResultTest, ExpectErr) {
  EXPECT_EQ((MakeErr<int, int>(20).expect_err("===TEST ERR MSG===")), 20);
  EXPECT_ANY_THROW((MakeOk<int, int>(10).expect_err("===TEST ERR MSG===")));

  EXPECT_EQ((MakeErr<vector<int>, int>(-40).expect_err("===TEST ERR MSG===")),
            -40);
  EXPECT_ANY_THROW((MakeOk<vector<int>, int>(vector{10, 20, 30})
                        .expect_err("===TEST ERR MSG===")));

  EXPECT_EQ((MakeErr<int, vector<int>>(vector{1, 2, 3, 4})
                 .expect_err("===TEST ERR MSG===")),
            (vector{1, 2, 3, 4}));
  EXPECT_ANY_THROW(
      (MakeOk<int, vector<int>>(68).expect_err("===TEST ERR MSG===")));

  EXPECT_EQ((MakeErr<vector<int>, vector<int>>(vector{1, 2, 3, 4})
                 .expect_err("===TEST ERR MSG===")),
            (vector{1, 2, 3, 4}));
  EXPECT_ANY_THROW((MakeOk<vector<int>, vector<int>>(vector{1, 2, 3, 4})
                        .expect_err("===TEST ERR MSG===")));
}

TEST(ResultTest, UnwapOrDefault) {
  EXPECT_EQ((MakeOk<int, int>(9).unwrap_or_default()), 9);
  EXPECT_EQ((MakeErr<int, int>(-9).unwrap_or_default()), 0);

  EXPECT_EQ(
      (MakeOk<vector<int>, int>(vector{1, 2, 3, 4, 5}).unwrap_or_default()),
      (vector{1, 2, 3, 4, 5}));
  EXPECT_EQ((MakeErr<vector<int>, int>(9).unwrap_or_default()),
            (vector<int>{}));

  EXPECT_EQ((MakeOk<int, vector<int>>(9).unwrap_or_default()), 9);
  EXPECT_EQ((MakeErr<int, vector<int>>(vector{-1, -2, -3, -4, -5})
                 .unwrap_or_default()),
            0);

  EXPECT_EQ((MakeOk<vector<int>, vector<int>>(vector{1, 2, 3, 4, 5})
                 .unwrap_or_default()),
            (vector{1, 2, 3, 4, 5}));
  EXPECT_EQ((MakeErr<vector<int>, vector<int>>(vector{-1, -2, -3, -4, -5})
                 .unwrap_or_default()),
            (vector<int>{}));
}

TEST(ResultTest, AsConstDeref) {
  int x = 99;
  auto const a = MakeOk<int*, int>(&x);
  EXPECT_EQ(a.as_const_deref().unwrap().get(), 99);
  EXPECT_EQ(&a.as_const_deref().unwrap().get(), &x);

  auto const b = MakeErr<int*, int>(-68);
  EXPECT_ANY_THROW(b.as_const_deref().unwrap().get());

  vector<int> vec{1, 2, 3, 4, 5};
  auto const c = MakeOk<typename vector<int>::iterator, int>(vec.begin());
  EXPECT_EQ(c.as_const_deref().unwrap().get(), 1);

  auto const d =
      MakeOk<typename vector<int>::const_reverse_iterator, int>(vec.crbegin());
  EXPECT_EQ(d.as_const_deref().unwrap().get(), vec.back());
  EXPECT_EQ(&d.as_const_deref().unwrap().get(), &vec.back());

  auto const e =
      MakeErr<typename vector<int>::const_reverse_iterator, int>(-87);
  EXPECT_ANY_THROW(e.as_const_deref().unwrap().get());
}

TEST(ResultTest, AsConstDerefErr) {
  int x = 99;
  auto const a = MakeErr<int, int*>(&x);
  EXPECT_EQ(a.as_const_deref_err().unwrap_err().get(), 99);
  EXPECT_EQ(&a.as_const_deref_err().unwrap_err().get(), &x);

  auto const b = MakeOk<int, int*>(-68);
  EXPECT_ANY_THROW(b.as_const_deref_err().unwrap_err().get());

  vector<int> vec{1, 2, 3, 4, 5};
  auto const c = MakeErr<int, typename vector<int>::iterator>(vec.begin());
  EXPECT_EQ(c.as_const_deref_err().unwrap_err().get(), 1);

  auto const d =
      MakeErr<int, typename vector<int>::const_reverse_iterator>(vec.crbegin());
  EXPECT_EQ(d.as_const_deref_err().unwrap_err().get(), vec.back());
  EXPECT_EQ(&d.as_const_deref_err().unwrap_err().get(), &vec.back());

  auto const e = MakeOk<int, typename vector<int>::const_reverse_iterator>(-87);
  EXPECT_ANY_THROW(e.as_const_deref_err().unwrap_err().get());
}

TEST(ResultTest, AsMutDeref) {
  int x = 99;
  auto a = MakeOk<int*, int>(&x);
  a.as_mut_deref().unwrap().get() = 1024;
  EXPECT_EQ(a.as_mut_deref().unwrap().get(), 1024);
  EXPECT_EQ(&a.as_mut_deref().unwrap().get(), &x);

  auto b = MakeErr<int*, int>(-68);
  EXPECT_ANY_THROW(b.as_mut_deref().unwrap().get());

  vector<int> vec{1, 2, 3, 4, 5};
  auto c = MakeOk<typename vector<int>::iterator, int>(vec.begin());
  c.as_mut_deref().unwrap().get() = 889;
  EXPECT_EQ(c.as_mut_deref().unwrap().get(), 889);

  auto d = MakeOk<typename vector<int>::reverse_iterator, int>(vec.rbegin());
  EXPECT_EQ(d.as_mut_deref().unwrap().get(), vec.back());
  EXPECT_EQ(&d.as_mut_deref().unwrap().get(), &vec.back());

  auto e = MakeErr<typename vector<int>::reverse_iterator, int>(-87);
  EXPECT_ANY_THROW(e.as_mut_deref().unwrap().get());
}

TEST(ResultTest, AsMutDerefErr) {
  int x = 99;
  auto a = MakeErr<int, int*>(&x);
  a.as_mut_deref_err().unwrap_err().get() = 1024;
  EXPECT_EQ(a.as_mut_deref_err().unwrap_err().get(), 1024);
  EXPECT_EQ(&a.as_mut_deref_err().unwrap_err().get(), &x);

  auto b = MakeOk<int, int*>(-68);
  EXPECT_ANY_THROW(b.as_mut_deref_err().unwrap_err().get());

  vector<int> vec{1, 2, 3, 4, 5};
  auto c = MakeErr<int, typename vector<int>::iterator>(vec.begin());
  c.as_mut_deref_err().unwrap_err().get() = 889;
  EXPECT_EQ(c.as_mut_deref_err().unwrap_err().get(), 889);

  auto d = MakeErr<int, typename vector<int>::reverse_iterator>(vec.rbegin());
  EXPECT_EQ(d.as_mut_deref_err().unwrap_err().get(), vec.back());
  EXPECT_EQ(&d.as_mut_deref_err().unwrap_err().get(), &vec.back());

  auto e = MakeOk<int, typename vector<int>::reverse_iterator>(-87);
  EXPECT_ANY_THROW(e.as_mut_deref_err().unwrap_err().get());
}

TEST(ResultTest, Match) {
  auto a = MakeOk<int, int>(98).match([](auto ok) { return ok + 2; },
                                      [](auto err) { return err + 5; });

  EXPECT_EQ(a, 100);

  auto b =
      MakeOk<vector<int>, int>(vector{1, 2, 3, 4, 5})
          .match([](auto ok) { return accumulate(ok.begin(), ok.end(), 0); },
                 [](auto) { return -1; });

  EXPECT_EQ(b, 15);

  auto c = MakeErr<vector<int>, int>(67).match(
      [](auto ok) { return accumulate(ok.begin(), ok.end(), 0); },
      [](auto) { return -1; });

  EXPECT_EQ(c, -1);
}

TEST(ResultTest, Clone) {
  auto const a = MakeOk<int, int>(89);
  auto b = a.clone();
  EXPECT_EQ(b, Ok(89));
  b.as_mut_ref().unwrap().get() = 124;
  EXPECT_EQ(b, Ok(124));
  EXPECT_EQ(a, Ok(89));

  auto const c = MakeOk<vector<int>, int>({89, 89, 120});
  auto d = c.clone();
  EXPECT_EQ(c, Ok(vector<int>{89, 89, 120}));
  d.as_mut_ref().unwrap().get() = vector<int>{124, 125, 126};
  EXPECT_EQ(d, Ok(vector<int>{124, 125, 126}));
  EXPECT_EQ(c, Ok(vector<int>{89, 89, 120}));
}

TEST(ResultTest, Docs) {
  
}