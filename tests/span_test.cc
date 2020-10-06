/**
 * @file span_test.cc
 * @author Basit Ayantunde <rlamarrr@gmail.com>
 * @date 2020-08-04
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

#include "stx/span.h"

#include <array>
#include <utility>

#include "gtest/gtest.h"

using namespace std;
using namespace string_literals;
using namespace stx;

static_assert(internal::is_container<std::array<int, 8>>);
static_assert(!internal::is_container<int>);
static_assert(!internal::is_container<int*>);
static_assert(!internal::is_container<int[6]>);
static_assert(internal::is_container<vector<int>>);

TEST(SpanTest, CopyConstructor) {
  vector<int> vec{1, 2, 3, 4, 5};

  {
    auto a = Span<int, 5>(vec);

    Span<volatile int, 3> b = a;

    Span<int, 1> c = a;

    Span<int, 5> d = Span<int>(vec);

    Span<int, 5> e = a;

    auto f = Span<int, 8>::try_init(Span<int>(vec));

    EXPECT_EQ(a.data(), vec.data());
    EXPECT_EQ(a.data(), b.data());
    EXPECT_EQ(b.data(), c.data());
    EXPECT_EQ(c.data(), d.data());
    EXPECT_EQ(d.data(), e.data());
    EXPECT_EQ(f, None);

    EXPECT_EQ(a.size(), vec.size());
    EXPECT_EQ(b.size(), 3);
    EXPECT_EQ(c.size(), 1);
    EXPECT_EQ(d.size(), 5);
    EXPECT_EQ(e.size(), a.size());
  }

  {
    auto a = Span<int>(vec);

    Span<int const> b = a;

    Span<int> c = a;

    Span<int> d = Span<int, 7>(vec);
    Span<int volatile> e = Span<int, 7>(vec);

    Span<int volatile> f = a;

    EXPECT_EQ(a.data(), vec.data());
    EXPECT_EQ(a.data(), b.data());
    EXPECT_EQ(b.data(), c.data());
    EXPECT_EQ(c.data(), d.data());
    EXPECT_EQ(d.data(), e.data());

    EXPECT_EQ(a.size(), vec.size());
    EXPECT_EQ(b.size(), a.size());
    EXPECT_EQ(c.size(), a.size());
    EXPECT_EQ(d.size(), 7);
    EXPECT_EQ(e.size(), 7);
    EXPECT_EQ(f.size(), a.size());
  }
}

TEST(DynamicSpanTest, ConstructorIterator) {
  int tmp[] = {1, 2, 3, 4};
  {
    Span<int, size(tmp)> a{tmp};
    Span<int, 3> b{tmp};
    EXPECT_EQ(a.data(), tmp);
    EXPECT_EQ(a.size(), size(tmp));

    EXPECT_EQ(b.data(), tmp);
    EXPECT_EQ(b.size(), 3);
  }

  {
    Span<int> a{nullptr, nullptr};
    Span<int> b{tmp, tmp + size(tmp)};
    EXPECT_EQ(a.data(), nullptr);
    EXPECT_EQ(a.size(), 0);

    EXPECT_EQ(b.data(), tmp);
    EXPECT_EQ(b.size(), size(tmp));

    EXPECT_NE(Span<int>::try_init(tmp, tmp + size(tmp)), None);
    EXPECT_EQ(Span<int>::try_init(tmp + size(tmp), tmp), None);
  }
}

TEST(SpanTest, ConstructorCArray) {
  int tmp[] = {1, 2, 3, 4};

  {
    Span<int, 3> b = tmp;

    EXPECT_EQ(b.data(), tmp);
    EXPECT_EQ(b.size(), 3);

    auto c = Span<int, 5>::try_init(tmp);

    EXPECT_EQ(c, None);

    auto d = Span<int, 4>::try_init(tmp);

    EXPECT_NE(d, None);
  }

  {
    Span<int> a = tmp;

    EXPECT_EQ(a.data(), tmp);
    EXPECT_EQ(a.size(), size(tmp));
  }
}

TEST(SpanTest, ConstructorStdArray) {
  array<int, 4> tmp{1, 2, 3, 4};
  {
    Span<int> a = tmp;

    EXPECT_EQ(a.data(), tmp.data());
    EXPECT_EQ(a.size(), size(tmp));
  }

  {
    Span<int, 3> b = tmp;

    EXPECT_EQ(b.data(), tmp.data());
    EXPECT_EQ(b.size(), 3);

    auto c = Span<int, 5>::try_init(tmp);
    EXPECT_EQ(c, None);

    auto d = Span<int, 4>::try_init(tmp);
    EXPECT_NE(d, None);
  }
}

TEST(SpanTest, ConstructorContainer) {
  vector<int> vec{1, 2, 3, 4, 5};
  string str = "12345";

  auto test_fn = [](auto vec) {
    using T = typename decltype(vec)::value_type;
    {
      Span<T> a(vec);

      Span<volatile T> b = a;
      Span<T const> c = a;
      Span<volatile const T> d = c;
      Span<volatile const T> e = b;

      EXPECT_EQ(a.data(), vec.data());
      EXPECT_EQ(a.data(), b.data());
      EXPECT_EQ(b.data(), c.data());
      EXPECT_EQ(c.data(), d.data());
      EXPECT_EQ(d.data(), e.data());

      EXPECT_EQ(a.size(), vec.size());
      EXPECT_EQ(a.size(), b.size());
      EXPECT_EQ(b.size(), c.size());
      EXPECT_EQ(c.size(), d.size());
      EXPECT_EQ(d.size(), e.size());
    }

    {
      Span<T, 3> a = vec;

      Span<volatile T, 3> b = a;
      Span<T const, 3> c = a;
      Span<volatile const T, 3> d = c;
      Span<volatile const T, 3> e = b;
      auto f = Span<T, 10>::try_init(vec);
      auto g = Span<T, 2>::try_init(vec);
      auto h = Span<T, 5>::try_init(vec);

      EXPECT_EQ(a.data(), vec.data());
      EXPECT_EQ(a.data(), b.data());
      EXPECT_EQ(b.data(), c.data());
      EXPECT_EQ(c.data(), d.data());
      EXPECT_EQ(d.data(), e.data());
      EXPECT_EQ(f, None);
      EXPECT_NE(g, None);
      EXPECT_NE(h, None);

      EXPECT_EQ(a.size(), 3);
      EXPECT_EQ(a.size(), b.size());
      EXPECT_EQ(b.size(), c.size());
      EXPECT_EQ(c.size(), d.size());
      EXPECT_EQ(d.size(), e.size());
    }
  };

  test_fn(vec);
  test_fn(str);
}

TEST(SpanTest, Empty) {
  int tmp[] = {1, 2, 3, 4};
  {
    Span<int, 0> a = tmp;
    EXPECT_TRUE(a.empty());
  }
  {
    Span<int> a(tmp, 0UL);
    EXPECT_TRUE(a.empty());
  }
}

TEST(SpanTest, At) {
  int tmp[] = {1, 2, 3, 4};

  {
    Span<int, size(tmp)> a = tmp;

    EXPECT_EQ(a.at(4), None);
    EXPECT_EQ(a.at(3), Some(4));

    EXPECT_EQ(a.at<4>(), None);
    EXPECT_EQ(a.at<3>(), Some(4));
  }

  {
    Span<int> a(tmp, size(tmp));

    EXPECT_EQ(a.at(4), None);
    EXPECT_EQ(a.at(3), Some(4));

    EXPECT_EQ(a.at<4>(), None);
    EXPECT_EQ(a.at<3>(), Some(4));
  }
}

TEST(SpanTest, As) {
  {
    int32_t tmp[] = {1, 2, 3, 4};
    Span<int32_t> a(tmp);
    Span<int32_t const> tmp_a = a.as_const();
    Span<byte> tmp_b = a.as_bytes();
    Span<uint8_t> tmp_c = a.as_u8();
    Span<volatile int32_t> tmp_d = a.as_volatile();

    Span<byte const volatile> b = a.as_u8().as_volatile().as_const().as_bytes();

    EXPECT_EQ(reinterpret_cast<byte const volatile*>(a.data()), b.data());
    EXPECT_EQ(a.size_bytes(), b.size());
    EXPECT_EQ(a.size() * 4, b.size());
  }

  {
    int32_t tmp[] = {1, 2, 3, 4};
    Span<int32_t, 3> a(tmp);
    Span<int32_t const> tmp_a = a.as_const();
    Span<byte, 12> tmp_b = a.as_bytes();
    Span<uint8_t, 12> tmp_c = a.as_u8();
    Span<volatile int32_t> tmp_d = a.as_volatile();

    Span<byte const volatile> e = a.as_u8().as_volatile().as_const().as_bytes();
  }
}

struct result_t {
  size_t size;
  int value;
};

namespace t1 {
constexpr result_t StaticSpan_Subspan(size_t index) {
  int tmp[] = {1, 2, 3, 4};

  Span<int, 4> h = tmp;

  return {h.subspan(index).size(), h.subspan(index)[0]};
}

constexpr result_t DynamicSpan_Subspan(size_t index) {
  int tmp[] = {1, 2, 3, 4};

  Span<int> h = tmp;

  return {h.subspan(index).size(), h.subspan(index)[0]};
}

}  // namespace t1

TEST(SpanTest, Subspan) {
  using namespace t1;
  {
    constexpr auto a = StaticSpan_Subspan(0);
    EXPECT_EQ(a.size, 4);
    EXPECT_EQ(a.value, 1);
  }

  {
    constexpr auto a = StaticSpan_Subspan(3);
    EXPECT_EQ(a.size, 1);
    EXPECT_EQ(a.value, 4);
  }

  {
    constexpr auto a = DynamicSpan_Subspan(0);
    EXPECT_EQ(a.size, 4);
    EXPECT_EQ(a.value, 1);
  }

  {
    constexpr auto a = DynamicSpan_Subspan(3);
    EXPECT_EQ(a.size, 1);
    EXPECT_EQ(a.value, 4);
  }
}

namespace cxpr1 {
template <size_t Offset>
constexpr result_t StaticSpan_Subspan() {
  int tmp[] = {1, 2, 3, 4};

  Span<int, 4> h = tmp;
  Span<int, 4 - Offset> i = h.subspan<Offset>();

  return {i.size(), i[0]};
}

template <size_t Offset>
constexpr result_t DynamicSpan_Subspan() {
  int tmp[] = {1, 2, 3, 4};

  Span<int> h = tmp;

  return {h.subspan<Offset>().size(), h.subspan<Offset>()[0]};
}
}  // namespace cxpr1

TEST(SpanTest, ConstexprSubspan) {
  using namespace cxpr1;
  {
    constexpr auto a = StaticSpan_Subspan<0>();
    EXPECT_EQ(a.size, 4);
    EXPECT_EQ(a.value, 1);
  }

  {
    constexpr auto a = StaticSpan_Subspan<3>();
    EXPECT_EQ(a.size, 1);
    EXPECT_EQ(a.value, 4);
  }

  {
    constexpr auto a = DynamicSpan_Subspan<0>();
    EXPECT_EQ(a.size, 4);
    EXPECT_EQ(a.value, 1);
  }

  {
    constexpr auto a = DynamicSpan_Subspan<3>();
    EXPECT_EQ(a.size, 1);
    EXPECT_EQ(a.value, 4);
  }
}

namespace cxpr2 {

struct result_t {
  int first;
  int last;
  size_t size;
};

template <size_t Offset, size_t Length>
constexpr result_t StaticSpan_Subspan() {
  int tmp[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

  Span<int, size(tmp)> a = tmp;
  Span<int, Length> b = a.subspan<Offset, Length>();

  return {b[0], b[b.size() - 1], b.size()};
}

template <size_t Offset, size_t Length>
constexpr result_t DynamicSpan_Subspan() {
  int tmp[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

  Span<int> a = tmp;
  Span<int, Length> b = a.subspan(Offset, Length);

  return {b[0], b[b.size() - 1], b.size()};
}
}  // namespace cxpr2

TEST(SpanTest, ConstexprSubspanWithLength) {
  using namespace cxpr2;
  {
    constexpr auto a = StaticSpan_Subspan<0, 3>();
    EXPECT_EQ(a.first, 1);
    EXPECT_EQ(a.last, 3);
    EXPECT_EQ(a.size, 3);
  }

  {
    constexpr auto a = StaticSpan_Subspan<6, 4>();
    EXPECT_EQ(a.first, 7);
    EXPECT_EQ(a.last, 10);
    EXPECT_EQ(a.size, 4);
  }

  {
    constexpr auto a = DynamicSpan_Subspan<0, 3>();
    EXPECT_EQ(a.first, 1);
    EXPECT_EQ(a.last, 3);
    EXPECT_EQ(a.size, 3);
  }

  {
    constexpr auto a = DynamicSpan_Subspan<6, 4>();
    EXPECT_EQ(a.first, 7);
    EXPECT_EQ(a.last, 10);
    EXPECT_EQ(a.size, 4);
  }
}
