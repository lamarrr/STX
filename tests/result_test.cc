/**
 * @file result_test.cc
 * @author Basit Ayantunde <rlamarrr@gmail.com>
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

#include "stx/result.h"

#include <numeric>
#include <algorithm>

#include "gtest/gtest.h"

using namespace std;
using namespace string_literals;
using namespace stx;

TEST(ResultTest, Equality) {
  //
  EXPECT_EQ((make_ok<int, int>(78)), Ok(78));
  EXPECT_NE((make_ok<int, int>(7)), Ok(78));
  EXPECT_NE((make_ok<int, int>(78)), Err(78));

  EXPECT_NE((make_err<int, int>(78)), Ok(78));
  EXPECT_NE((make_err<int, int>(7)), Ok(78));
  EXPECT_NE((make_err<int, int>(78)), Err(-78));
  EXPECT_EQ((make_err<int, int>(78)), Err(78));

  EXPECT_EQ((make_ok<vector<int>, int>(vector{1, 2, 3, 4, 5})),
            Ok(vector{1, 2, 3, 4, 5}));
  EXPECT_EQ((make_ok<vector<int>, int>(vector{1, 2, 3, 4, 5})),
            (make_ok<vector<int>, int>(vector{1, 2, 3, 4, 5})));

  EXPECT_NE((make_ok<vector<int>, vector<int>>(vector{1, 2, 3, 4, 5})),
            Err(vector{1, 2, 3, 4, 5}));
  EXPECT_NE((make_ok<vector<int>, vector<int>>(vector{1, 2, 3, 4, 5})),
            (make_err<vector<int>, vector<int>>(vector{1, 2, 3, 4, 5})));

  int const f = 89;
  int g = 89;
  EXPECT_EQ(Ok(89), ok_ref(f));
  EXPECT_EQ(Ok(89), ok_ref(g));

  EXPECT_NE(Ok(8), ok_ref(f));
  EXPECT_NE(Ok(8), ok_ref(g));

  EXPECT_EQ(Ok(89), ok_ref(g));
  EXPECT_EQ(Ok(89), ok_ref(f));

  EXPECT_NE(Ok(9), ok_ref(g));
  EXPECT_NE(Ok(9), ok_ref(f));

  int const h = 89;
  int i = 89;

  EXPECT_EQ(Err(89), err_ref(h));
  EXPECT_EQ(Err(89), err_ref(i));

  EXPECT_NE(Err(8), err_ref(h));
  EXPECT_NE(Err(8), err_ref(i));

  EXPECT_EQ(Err(89), err_ref(h));
  EXPECT_EQ(Err(89), err_ref(i));

  EXPECT_NE(Err(8), err_ref(h));
  EXPECT_NE(Err(8), err_ref(i));

  EXPECT_EQ((make_ok<int, int>(89)), ok_ref(h));
  EXPECT_EQ((make_ok<int, int>(89)), ok_ref(i));

  EXPECT_EQ((make_ok<int, int>(89)), ok_ref(h));
  EXPECT_EQ((make_ok<int, int>(89)), ok_ref(i));

  EXPECT_EQ((make_ok<int, int>(89)), ok_ref(i));

  EXPECT_EQ((make_err<int, int>(89)), err_ref(h));
  EXPECT_EQ((make_err<int, int>(89)), err_ref(i));

  EXPECT_EQ((make_err<int, int>(89)), err_ref(h));
  EXPECT_EQ((make_err<int, int>(89)), err_ref(i));

  EXPECT_EQ((make_err<int, int>(89)), err_ref(i));
}

TEST(ResultTest, IsOk) {
  EXPECT_TRUE((make_ok<int, int>(0).is_ok()));
  EXPECT_FALSE((make_err<int, int>(9).is_ok()));

  EXPECT_TRUE((make_ok<vector<int>, int>(vector{1, 2, 3, 4}).is_ok()));
  EXPECT_FALSE((make_err<vector<int>, int>(89).is_ok()));

  EXPECT_TRUE((make_ok<int, vector<int>>(-78).is_ok()));
  EXPECT_FALSE((make_err<int, vector<int>>(vector{1, 2, 3, 4}).is_ok()));

  EXPECT_TRUE((make_ok<vector<int>, vector<int>>(vector{1, 2, 3, 4}).is_ok()));
  EXPECT_FALSE(
      (make_err<vector<int>, vector<int>>(vector{1, 2, 3, 4}).is_ok()));
}

TEST(ResultTest, IsErr) {
  EXPECT_TRUE((make_err<int, int>(9).is_err()));
  EXPECT_FALSE((make_ok<int, int>(0).is_err()));

  EXPECT_TRUE((make_err<vector<int>, int>(89).is_err()));
  EXPECT_FALSE((make_ok<vector<int>, int>(vector{1, 2, 3, 4}).is_err()));

  EXPECT_TRUE((make_err<int, vector<int>>(vector{1, 2, 3, 4}).is_err()));
  EXPECT_FALSE((make_ok<int, vector<int>>(99).is_err()));

  EXPECT_TRUE(
      (make_err<vector<int>, vector<int>>(vector{5, 6, 7, 8}).is_err()));
  EXPECT_FALSE(
      (make_ok<vector<int>, vector<int>>(vector{1, 2, 3, 4}).is_err()));
}

TEST(ResultTest, Contains) {
  EXPECT_TRUE((make_ok<int, int>(9).contains(9)));
  EXPECT_FALSE((make_ok<int, int>(10).contains(0)));
  EXPECT_FALSE((make_err<int, int>(0).contains(0)));

  EXPECT_TRUE(
      (make_ok<vector<int>, int>({1, 2, 3, 4}).contains(vector{1, 2, 3, 4})));
  EXPECT_FALSE(
      (make_ok<vector<int>, int>({1, 2, 3, 4}).contains(vector{5, 6, 7, 8})));
  EXPECT_FALSE((make_err<vector<int>, int>(89).contains(vector{1, 2, 3, 4})));

  EXPECT_TRUE((make_ok<int, vector<int>>(9).contains(9)));
  EXPECT_FALSE((make_ok<int, vector<int>>(10).contains(0)));
  EXPECT_FALSE((make_err<int, vector<int>>(vector{4, 5, 6, 7, 8}).contains(0)));

  EXPECT_TRUE((make_ok<vector<int>, vector<int>>(vector{1, 2, 3, 4})
                   .contains(vector{1, 2, 3, 4})));
  EXPECT_FALSE((make_ok<vector<int>, vector<int>>(vector{1, 2, 3, 4})
                    .contains(vector{5, 6, 7, 8})));
  EXPECT_FALSE((make_err<vector<int>, vector<int>>(vector{5, 6, 7, 8, 9})
                    .contains(vector{1, 2, 3, 4})));
}

TEST(ResultTest, ContainsErr) {
  EXPECT_TRUE((make_err<int, int>(0).contains_err(0)));
  EXPECT_FALSE((make_err<int, int>(9).contains_err(20)));
  EXPECT_FALSE((make_ok<int, int>(10).contains_err(10)));

  EXPECT_TRUE((make_err<vector<int>, int>(0).contains_err(0)));
  EXPECT_FALSE((make_err<vector<int>, int>(9).contains_err(20)));
  EXPECT_FALSE(
      (make_ok<vector<int>, int>(vector{1, 2, 3, 4}).contains_err(10)));

  EXPECT_TRUE((make_err<int, vector<int>>(vector{1, 2, 3, 4})
                   .contains_err(vector{1, 2, 3, 4})));
  EXPECT_FALSE((make_err<int, vector<int>>(vector{1, 2, 3, 4})
                    .contains_err(vector{5, 6, 7, 8})));
  EXPECT_FALSE(
      (make_ok<int, vector<int>>(89).contains_err(vector{1, 2, 3, 4})));

  EXPECT_TRUE((make_err<vector<int>, vector<int>>(vector{1, 2, 3, 4})
                   .contains_err(vector{1, 2, 3, 4})));
  EXPECT_FALSE((make_err<vector<int>, vector<int>>(vector{1, 2, 3, 4})
                    .contains_err(vector{5, 6, 7, 8})));
  EXPECT_FALSE((make_ok<vector<int>, vector<int>>(vector{1, 2, 3, 4})
                    .contains_err(vector{1, 2, 3, 4})));
}

TEST(ResultTest, Exists) {
  auto even = [](auto x) { return x % 2 == 0; };
  auto all_even = [=](auto x) { return std::all_of(x.begin(), x.end(), even); };

  EXPECT_TRUE((make_ok<int, int>(2).exists(even)));
  EXPECT_FALSE((make_ok<int, int>(5).exists(even)));
  EXPECT_FALSE((make_err<int, int>(2).exists(even)));
  EXPECT_FALSE((make_err<int, int>(3).exists(even)));

  EXPECT_TRUE(
      (make_ok<vector<int>, int>(vector{2, 4, 6, 8, 10, 12}).exists(all_even)));
  EXPECT_FALSE((make_ok<vector<int>, int>(vector{2, 4, 6, 8, 10, 12, 13})
                    .exists(all_even)));
  EXPECT_FALSE((make_err<vector<int>, int>(2).exists(all_even)));
  EXPECT_FALSE((make_err<vector<int>, int>(3).exists(all_even)));

  EXPECT_TRUE((make_ok<int, vector<int>>(2).exists(even)));
  EXPECT_FALSE((make_ok<int, vector<int>>(5).exists(even)));
  EXPECT_FALSE(
      (make_err<int, vector<int>>(vector{2, 4, 6, 8, 10, 12}).exists(even)));
  EXPECT_FALSE((
      make_err<int, vector<int>>(vector{2, 4, 6, 8, 10, 12, 13}).exists(even)));

  EXPECT_TRUE((make_ok<vector<int>, vector<int>>(vector{2, 4, 6, 8, 10, 12})
                   .exists(all_even)));
  EXPECT_FALSE(
      (make_ok<vector<int>, vector<int>>(vector{2, 4, 6, 8, 10, 12, 13})
           .exists(all_even)));
  EXPECT_FALSE((make_err<vector<int>, vector<int>>(vector{2, 4, 6, 8, 10, 12})
                    .exists(all_even)));
  EXPECT_FALSE(
      (make_err<vector<int>, vector<int>>(vector{2, 4, 6, 8, 10, 12, 13})
           .exists(all_even)));
}

TEST(ResultTest, ErrExists) {
  auto even = [](auto x) { return x % 2 == 0; };
  auto all_even = [=](auto x) { return std::all_of(x.begin(), x.end(), even); };

  EXPECT_FALSE((make_ok<int, int>(2).err_exists(even)));
  EXPECT_FALSE((make_ok<int, int>(5).err_exists(even)));
  EXPECT_TRUE((make_err<int, int>(2).err_exists(even)));
  EXPECT_FALSE((make_err<int, int>(3).err_exists(even)));

  EXPECT_FALSE(
      (make_ok<vector<int>, int>(vector{2, 4, 6, 8, 10, 12}).err_exists(even)));
  EXPECT_FALSE((make_ok<vector<int>, int>(vector{2, 4, 6, 8, 10, 12, 13})
                    .err_exists(even)));
  EXPECT_TRUE((make_err<vector<int>, int>(2).err_exists(even)));
  EXPECT_FALSE((make_err<vector<int>, int>(3).err_exists(even)));

  EXPECT_FALSE((make_ok<int, vector<int>>(2).err_exists(all_even)));
  EXPECT_FALSE((make_ok<int, vector<int>>(5).err_exists(all_even)));
  EXPECT_TRUE((make_err<int, vector<int>>(vector{2, 4, 6, 8, 10, 12})
                   .err_exists(all_even)));
  EXPECT_FALSE((make_err<int, vector<int>>(vector{2, 4, 6, 8, 10, 12, 13})
                    .err_exists(all_even)));

  EXPECT_FALSE((make_ok<vector<int>, vector<int>>(vector{2, 4, 6, 8, 10, 12})
                    .err_exists(all_even)));
  EXPECT_FALSE(
      (make_ok<vector<int>, vector<int>>(vector{2, 4, 6, 8, 10, 12, 13})
           .err_exists(all_even)));
  EXPECT_TRUE((make_err<vector<int>, vector<int>>(vector{2, 4, 6, 8, 10, 12})
                   .err_exists(all_even)));
  EXPECT_FALSE(
      (make_err<vector<int>, vector<int>>(vector{2, 4, 6, 8, 10, 12, 13})
           .err_exists(all_even)));
}

TEST(ResultTest, Ok) {
  EXPECT_EQ((make_ok<int, int>(20).ok().unwrap()), 20);
  EXPECT_EQ((make_err<int, int>(90).ok()), None);

  EXPECT_EQ((make_ok<vector<int>, int>(vector{1, 2, 3, 4}).ok().unwrap()),
            (vector{1, 2, 3, 4}));
  EXPECT_EQ((make_err<vector<int>, int>(90).ok()), None);

  EXPECT_EQ((make_ok<int, vector<int>>(90).ok().unwrap()), 90);
  EXPECT_EQ((make_err<int, vector<int>>(vector{1, 2, 3, 4}).ok()), None);

  EXPECT_EQ(
      (make_ok<vector<int>, vector<int>>(vector{1, 2, 3, 4}).ok().unwrap()),
      (vector{1, 2, 3, 4}));
  EXPECT_EQ((make_err<vector<int>, vector<int>>(vector{1, 2, 3, 4}).ok()),
            None);
}

TEST(ResultTest, Err) {
  EXPECT_EQ((make_err<int, int>(20).err().unwrap()), 20);
  EXPECT_EQ((make_ok<int, int>(90).err()), None);

  EXPECT_EQ((make_err<vector<int>, int>(10).err().unwrap()), 10);
  EXPECT_EQ((make_ok<vector<int>, int>(vector{1, 2, 3, 4}).err()), None);

  EXPECT_EQ((make_err<int, vector<int>>(vector{1, 2, 3, 4}).err().unwrap()),
            (vector{1, 2, 3, 4}));
  EXPECT_EQ((make_ok<int, vector<int>>(78).err()), None);

  EXPECT_EQ(
      (make_err<vector<int>, vector<int>>(vector{1, 2, 3, 4}).err().unwrap()),
      (vector{1, 2, 3, 4}));
  EXPECT_EQ((make_ok<vector<int>, vector<int>>(vector{78, 67}).err()), None);
}

TEST(ResultTest, AsConstRef) {
  auto const a = make_ok<int, int>(68);
  EXPECT_EQ(a.as_cref().unwrap().get(), 68);
  auto const b = make_err<int, int>(-68);
  EXPECT_TRUE(b.as_cref().is_err());

  auto const c = make_ok<vector<int>, int>(vector{1, 2, 3, 4});
  EXPECT_EQ(c.as_cref().unwrap().get(), (vector{1, 2, 3, 4}));
  auto const d = make_err<vector<int>, int>(-24);
  EXPECT_TRUE(d.as_cref().is_err());

  auto const e = make_ok<int, vector<int>>(-24);
  EXPECT_EQ(e.as_cref().unwrap().get(), -24);
  auto const f = make_err<int, vector<int>>(vector{-1, -2, -3, -4});
  EXPECT_TRUE(f.as_cref().is_err());

  auto const g = make_ok<vector<int>, vector<int>>(vector{1, 2, 3, 4});
  EXPECT_EQ(g.as_cref().unwrap().get(), (vector{1, 2, 3, 4}));
  auto const h = make_err<vector<int>, vector<int>>(vector{-1, -2, -3, -4});
  EXPECT_TRUE(h.as_cref().is_err());
}

TEST(ResultTest, AsMutRef) {
  auto a = make_ok<int, int>(68);
  a.as_ref().unwrap().get() = 89;
  EXPECT_EQ(a.as_ref().unwrap().get(), 89);

  auto b = make_err<int, int>(-68);
  EXPECT_TRUE(b.as_ref().is_err());
  EXPECT_EQ(b.as_ref().unwrap_err().get(), -68);

  auto c = make_ok<vector<int>, int>(vector{1, 2, 3, 4});
  c.as_ref().unwrap().get() = vector{-1, -2, -3, -4};
  EXPECT_EQ(c.as_ref().unwrap().get(), (vector{-1, -2, -3, -4}));

  auto d = make_err<vector<int>, int>(-24);
  EXPECT_TRUE(d.as_ref().is_err());
  EXPECT_EQ(d.as_ref().unwrap_err(), -24);

  auto e = make_ok<int, vector<int>>(-24);
  e.as_ref().unwrap().get() = 87;
  EXPECT_EQ(e.as_ref().unwrap().get(), 87);

  auto f = make_err<int, vector<int>>(vector{-1, -2, -3, -4});
  EXPECT_TRUE(f.as_ref().is_err());
  EXPECT_EQ(f.as_ref().unwrap_err().get(), (vector{-1, -2, -3, -4}));

  auto g = make_ok<vector<int>, vector<int>>(vector{1, 2, 3, 4});
  EXPECT_EQ(g.as_ref().unwrap().get(), (vector{1, 2, 3, 4}));
  EXPECT_TRUE(g.as_ref().is_ok());

  auto h = make_err<vector<int>, vector<int>>(vector{-1, -2, -3, -4});
  EXPECT_TRUE(h.as_ref().is_err());
  h.as_ref().unwrap_err().get() = vector{-5, -6, -7, -8, -9};
  EXPECT_EQ(h.as_ref().unwrap_err().get(), (vector{-5, -6, -7, -8, -9}));
}

TEST(ResultTest, Map) {
  // will it be problematic if the map function is not in scope? No!
  auto a = [](int&& value) { return value + 20; };
  EXPECT_EQ((make_ok<int, int>(20).map(a).unwrap()), 40);
  EXPECT_TRUE((make_err<int, int>(-1).map(a).is_err()));

  // will it be problematic if the map function is not in scope? No!
  auto b = [](vector<int>&& value) {
    value.push_back(6);
    return move(value);
  };
  EXPECT_EQ((make_ok<vector<int>, int>({1, 2, 3, 4, 5}).map(b).unwrap()),
            (vector{1, 2, 3, 4, 5, 6}));
  EXPECT_TRUE((make_err<vector<int>, int>(-1).map(b).is_err()));
}

TEST(ResultTest, MapOr) {
  auto a = [](int&& value) { return value + 20; };
  EXPECT_EQ((make_ok<int, int>(20).map_or(a, 100)), 40);
  EXPECT_EQ((make_err<int, int>(-20).map_or(a, 100)), 100);

  auto b = [](vector<int>&& value) {
    value.push_back(6);
    return move(value);
  };
  EXPECT_EQ(
      (make_ok<vector<int>, int>({1, 2, 3, 4, 5}).map_or(b, vector<int>{})),
      (vector<int>{1, 2, 3, 4, 5, 6}));
  EXPECT_EQ(
      (make_err<vector<int>, int>(-20).map_or(b, vector<int>{6, 7, 8, 9, 10})),
      (vector<int>{6, 7, 8, 9, 10}));
}

TEST(ResultTest, MapOrElse) {
  auto a = [](int&& value) { return value + 20; };
  auto else_a = [](auto) { return -10; };

  EXPECT_EQ((make_ok<int, int>(20).map_or_else(a, else_a)), 40);
  EXPECT_EQ((make_err<int, int>(-20).map_or_else(a, else_a)), -10);

  auto b = [](vector<int>&& value) {
    value.push_back(6);
    return move(value);
  };
  auto else_b = [](auto) -> vector<int> { return {6, 7, 8, 9, 10}; };  // NOLINT

  EXPECT_EQ((make_ok<vector<int>, int>({1, 2, 3, 4, 5}).map_or_else(b, else_b)),
            (vector<int>{1, 2, 3, 4, 5, 6}));
  EXPECT_EQ((make_err<vector<int>, int>(-20).map_or_else(b, else_b)),
            (vector<int>{6, 7, 8, 9, 10}));
}

TEST(ResultTest, MapErr) {
  // will it be problematic if the map function is not in scope? No!
  auto a = [](int&& value) { return value * 10; };
  EXPECT_EQ((make_err<int, int>(10).map_err(a).unwrap_err()), 100);
  EXPECT_TRUE((make_ok<int, int>(20).map_err(a).is_ok()));

  // will it be problematic if the map function is not in scope? No!
  auto b = [](vector<int>&& value) {
    value.push_back(6);
    return move(value);
  };
  EXPECT_EQ(
      (make_err<int, vector<int>>({1, 2, 3, 4, 5}).map_err(b).unwrap_err()),
      (vector{1, 2, 3, 4, 5, 6}));

  EXPECT_TRUE((make_ok<int, vector<int>>(256).map_err(b).is_ok()));
}

TEST(ResultTest, And) {
  EXPECT_FLOAT_EQ(
      (make_ok<int, int>(20).AND(make_ok<float, int>(40.0f)).unwrap()), 40.0f);

  EXPECT_TRUE(
      (make_err<int, int>(-20).AND(make_ok<float, int>(40.0f)).is_err()));

  EXPECT_EQ((make_ok<int, int>(20)
                 .AND(make_ok<vector<float>, int>({40.0f, 50.0f, 60.0f}))
                 .unwrap()),
            (vector{40.0f, 50.0f, 60.0f}));

  EXPECT_TRUE(
      (make_err<int, int>(-20)
           .AND(make_ok<vector<float>, int>(vector{40.0f, 50.0f, 60.0f}))
           .is_err()));

  EXPECT_EQ((make_err<int, int>(-20)
                 .AND(make_ok<vector<float>, int>(vector{40.0f, 50.0f, 60.0f}))
                 .unwrap_err()),
            -20);
}

TEST(ResultTest, AndThen) {
  auto a = [](int v) { return v * 2.0f; };
  EXPECT_FLOAT_EQ((make_ok<int, int>(20).and_then(a).unwrap()), 40.0f);

  EXPECT_TRUE((make_err<int, int>(-20).and_then(a).is_err()));

  EXPECT_EQ((make_err<int, int>(-20).and_then(a).unwrap_err()), -20);

  auto b = [](int&& v) {
    vector<float> res;
    res.push_back(static_cast<float>(v));
    return res;
  };

  EXPECT_EQ((make_ok<int, int>(80).and_then(b).unwrap()), (vector{80.0f}));

  EXPECT_TRUE((make_err<int, int>(-20).and_then(b).is_err()));
  EXPECT_EQ((make_err<int, int>(-20).and_then(b).unwrap_err()), -20);
}

TEST(ResultTest, Or) {
  EXPECT_EQ((make_ok<int, int>(20).OR(make_ok<int, int>(40)).unwrap()), 20);

  EXPECT_EQ((make_ok<int, int>(20).OR(make_err<int, int>(40)).unwrap()), 20);

  EXPECT_TRUE((make_err<int, int>(-20).OR(make_err<int, int>(40)).is_err()));

  EXPECT_EQ((make_err<int, int>(-20).OR(make_err<int, int>(40)).unwrap_err()),
            40);

  EXPECT_EQ((make_ok<vector<int>, int>(vector{1, 2, 3, 4, 5})
                 .OR(make_ok<vector<int>, int>(vector{40, 50, 60}))
                 .unwrap()),
            (vector{1, 2, 3, 4, 5}));

  EXPECT_EQ((make_ok<vector<int>, int>(vector{1, 2, 3, 4, 5})
                 .OR(make_err<vector<int>, int>(8))
                 .unwrap()),
            (vector{1, 2, 3, 4, 5}));

  EXPECT_TRUE((make_err<vector<int>, int>(-9)
                   .OR(make_err<vector<int>, int>(-8))
                   .is_err()));

  EXPECT_EQ((make_err<vector<float>, int>(-20)
                 .OR(make_ok<vector<float>, int>(vector{40.0f, 50.0f, 60.0f}))
                 .unwrap()),
            (vector{40.0f, 50.0f, 60.0f}));
}

TEST(ResultTest, OrElse) {
  auto a = [](int&& err) -> Result<int, int> { return Ok(err * 100); };
  EXPECT_EQ((make_ok<int, int>(20).or_else(a).unwrap()), 20);
  EXPECT_EQ((make_err<int, int>(10).or_else(a).unwrap()), 1000);

  auto b = [](string&& err) -> Result<int, string> {
    return Err("Err: " + err);
  };
  EXPECT_EQ((make_ok<int, string>(20).or_else(b).unwrap()), 20);
  EXPECT_EQ((make_err<int, string>("Max Limit"s).or_else(b).unwrap_err()),
            "Err: Max Limit"s);

  auto c = [](vector<int>&& err) -> Result<int, vector<int>> {
    return Ok(err.empty() ? -1 : err[0]);
  };
  EXPECT_EQ((make_ok<int, vector<int>>(40).or_else(c).unwrap()), 40);
  EXPECT_EQ(
      (make_err<int, vector<int>>(vector{10, 20, 30}).or_else(c).unwrap()), 10);
}

TEST(ResultTest, UnwrapOr) {
  EXPECT_EQ((make_ok<int, int>(89).unwrap_or(90)), 89);
  EXPECT_EQ((make_err<int, int>(89).unwrap_or(90)), 90);

  EXPECT_EQ((make_ok<string, int>("John Doe"s).unwrap_or("Unknown"s)),
            "John Doe"s);
  EXPECT_EQ((make_err<string, int>(-20).unwrap_or("Unknown"s)), "Unknown"s);
}

TEST(ResultTest, Unwrap) {
  EXPECT_EQ((make_ok<int, int>(89).unwrap()), 89);
  EXPECT_TRUE((make_err<int, int>(89).is_err()));

  EXPECT_EQ((make_ok<string, int>("John Doe"s).unwrap()), "John Doe"s);
  EXPECT_TRUE((make_err<string, int>(-20)).is_err());

  EXPECT_EQ((make_ok<vector<int>, int>(vector{1, 2, 3, 4, 5}).unwrap()),
            (vector{1, 2, 3, 4, 5}));
  EXPECT_TRUE((make_err<vector<int>, int>(-1)).is_err());
}

TEST(ResultTest, UnwrapOrElse) {
  auto a = [](int&& err) { return err + 20; };
  EXPECT_EQ((make_ok<int, int>(10).unwrap_or_else(a)), 10);
  EXPECT_EQ((make_err<int, int>(20).unwrap_or_else(a)), 40);

  auto b = [](string&& err) -> int { return stoi(err) + 20; };
  EXPECT_EQ((make_ok<int, string>(10).unwrap_or_else(b)), 10);
  EXPECT_EQ((make_err<int, string>("40"s).unwrap_or_else(b)), 60);

  auto c = [](vector<int>&& vec) {
    vec.push_back(10);
    return move(vec);
  };
  EXPECT_EQ((make_ok<vector<int>, vector<int>>(vector{1, 2, 3, 4, 5})
                 .unwrap_or_else(c)),
            (vector{1, 2, 3, 4, 5}));
  EXPECT_EQ((make_err<vector<int>, vector<int>>(vector{6, 7, 8, 9})
                 .unwrap_or_else(c)),
            (vector{6, 7, 8, 9, 10}));
}

TEST(ResultTest, Expect) {
  EXPECT_EQ((make_ok<int, int>(10).expect("===TEST ERR MSG===")), 10);
  EXPECT_DEATH_IF_SUPPORTED(
      (make_err<int, int>(20).expect("===TEST ERR MSG===")), ".*");

  EXPECT_EQ((make_ok<vector<int>, int>(vector{1, 2, 3, 4, 5})
                 .expect("===TEST ERR MSG===")),
            (vector{1, 2, 3, 4, 5}));
  EXPECT_DEATH_IF_SUPPORTED(
      (make_err<vector<int>, int>(20).expect("===TEST ERR MSG===")), ".*");

  EXPECT_EQ((make_ok<int, vector<int>>(-1).expect("===TEST ERR MSG===")), -1);
  EXPECT_DEATH_IF_SUPPORTED(
      (make_err<int, vector<int>>(vector{-1, -2, -3, -4, -5})
           .expect("===TEST ERR MSG===")),
      ".*");

  EXPECT_EQ((make_ok<vector<int>, vector<int>>(vector{1, 2, 3, 4, 5})
                 .expect("===TEST ERR MSG===")),
            (vector{1, 2, 3, 4, 5}));

  EXPECT_DEATH_IF_SUPPORTED(
      (make_err<vector<int>, vector<int>>(vector{-1, -2, -3, -4, -5})
           .expect("===TEST ERR MSG===")),
      ".*");
}

TEST(ResultTest, UnwrapErr) {
  EXPECT_EQ((make_err<int, int>(20).unwrap_err()), 20);
  EXPECT_DEATH_IF_SUPPORTED((make_ok<int, int>(10).unwrap_err()), ".*");

  EXPECT_EQ((make_err<vector<int>, int>(-40).unwrap_err()), -40);
  EXPECT_DEATH_IF_SUPPORTED(
      (make_ok<vector<int>, int>(vector{10, 20, 30}).unwrap_err()), ".*");

  EXPECT_EQ((make_err<int, vector<int>>(vector{1, 2, 3, 4}).unwrap_err()),
            (vector{1, 2, 3, 4}));
  EXPECT_DEATH_IF_SUPPORTED((make_ok<int, vector<int>>(68).unwrap_err()), ".*");

  EXPECT_EQ(
      (make_err<vector<int>, vector<int>>(vector{1, 2, 3, 4}).unwrap_err()),
      (vector{1, 2, 3, 4}));
  EXPECT_DEATH_IF_SUPPORTED(
      (make_ok<vector<int>, vector<int>>(vector{1, 2, 3, 4}).unwrap_err()),
      ".*");
}

TEST(ResultTest, ExpectErr) {
  EXPECT_EQ((make_err<int, int>(20).expect_err("===TEST ERR MSG===")), 20);
  EXPECT_DEATH_IF_SUPPORTED(
      (make_ok<int, int>(10).expect_err("===TEST ERR MSG===")), ".*");

  EXPECT_EQ((make_err<vector<int>, int>(-40).expect_err("===TEST ERR MSG===")),
            -40);
  EXPECT_DEATH_IF_SUPPORTED((make_ok<vector<int>, int>(vector{10, 20, 30})
                                 .expect_err("===TEST ERR MSG===")),
                            ".*");

  EXPECT_EQ((make_err<int, vector<int>>(vector{1, 2, 3, 4})
                 .expect_err("===TEST ERR MSG===")),
            (vector{1, 2, 3, 4}));
  EXPECT_DEATH_IF_SUPPORTED(
      (make_ok<int, vector<int>>(68).expect_err("===TEST ERR MSG===")), ".*");

  EXPECT_EQ((make_err<vector<int>, vector<int>>(vector{1, 2, 3, 4})
                 .expect_err("===TEST ERR MSG===")),
            (vector{1, 2, 3, 4}));
  EXPECT_DEATH_IF_SUPPORTED(
      (make_ok<vector<int>, vector<int>>(vector{1, 2, 3, 4})
           .expect_err("===TEST ERR MSG===")),
      ".*");
}

TEST(ResultTest, UnwapOrDefault) {
  EXPECT_EQ((make_ok<int, int>(9).unwrap_or_default()), 9);
  EXPECT_EQ((make_err<int, int>(-9).unwrap_or_default()), 0);

  EXPECT_EQ(
      (make_ok<vector<int>, int>(vector{1, 2, 3, 4, 5}).unwrap_or_default()),
      (vector{1, 2, 3, 4, 5}));
  EXPECT_EQ((make_err<vector<int>, int>(9).unwrap_or_default()),
            (vector<int>{}));

  EXPECT_EQ((make_ok<int, vector<int>>(9).unwrap_or_default()), 9);
  EXPECT_EQ((make_err<int, vector<int>>(vector{-1, -2, -3, -4, -5})
                 .unwrap_or_default()),
            0);

  EXPECT_EQ((make_ok<vector<int>, vector<int>>(vector{1, 2, 3, 4, 5})
                 .unwrap_or_default()),
            (vector{1, 2, 3, 4, 5}));
  EXPECT_EQ((make_err<vector<int>, vector<int>>(vector{-1, -2, -3, -4, -5})
                 .unwrap_or_default()),
            (vector<int>{}));
}

TEST(ResultTest, Match) {
  auto a = make_ok<int, int>(98).match([](auto ok) { return ok + 2; },
                                       [](auto err) { return err + 5; });

  EXPECT_EQ(a, 100);

  auto b =
      make_ok<vector<int>, int>(vector{1, 2, 3, 4, 5})
          .match([](auto ok) { return accumulate(ok.begin(), ok.end(), 0); },
                 [](auto) { return -1; });

  EXPECT_EQ(b, 15);

  auto c = make_err<vector<int>, int>(67).match(
      [](auto ok) { return accumulate(ok.begin(), ok.end(), 0); },
      [](auto) { return -1; });

  EXPECT_EQ(c, -1);
}

TEST(ResultTest, Clone) {
  auto const a = make_ok<int, int>(89);
  auto b = a.clone();
  EXPECT_EQ(b, Ok(89));
  b.as_ref().unwrap().get() = 124;
  EXPECT_EQ(b, Ok(124));
  EXPECT_EQ(a, Ok(89));

  auto const c = make_ok<vector<int>, int>({89, 89, 120});
  auto d = c.clone();
  EXPECT_EQ(c, Ok(vector<int>{89, 89, 120}));
  d.as_ref().unwrap().get() = vector<int>{124, 125, 126};
  EXPECT_EQ(d, Ok(vector<int>{124, 125, 126}));
  EXPECT_EQ(c, Ok(vector<int>{89, 89, 120}));
}

auto ok_try_b(int x) -> stx::Result<int, int> {
  if (x > 0) {
    return Ok(std::move(x));
  } else {
    return Err(-1);
  }
}

auto ok_try_a(int m) -> stx::Result<int, int> {
  // clang-format off
  TRY_OK(x, ok_try_b(m)); TRY_OK(const y, ok_try_b(m)); TRY_OK(volatile z, ok_try_b(m)); // also tests for name collision in our macros
  // clang-format on
  x += 60;
  return Ok(std::move(x));
}

TEST(ResultTest, TryOk) {
  EXPECT_EQ(ok_try_a(10), Ok(70));
  EXPECT_EQ(ok_try_a(100'000), Ok(100'060));
  EXPECT_EQ(ok_try_a(-1), Err(-1));
  EXPECT_EQ(ok_try_a(-10), Err(-1));
}

TEST(ResultTest, Docs) {}
