/**
 * @file span_test.cc
 * @author Basit Ayantunde <rlamarrr@gmail.com>
 * @date 2020-08-04
 *
 * @copyright MIT License
 *
 * Copyright (c) 2020-2022 Basit Ayantunde
 *
 */

#include "stx/span.h"

#include <array>
#include <utility>
#include <vector>

#include "gtest/gtest.h"

using namespace std;
using namespace string_literals;
using namespace stx;

static_assert(impl::is_span_convertible<int, volatile int>);
static_assert(impl::is_span_convertible<int, int const>);
static_assert(impl::is_compatible_container<std::vector<int> &, int const>);

TEST(SpanTest, ContainerConstructor)
{
  std::vector<int> a{1, 2, 3, 4, 5};
  Span             b = a;

  EXPECT_EQ(b.size(), a.size());
  EXPECT_EQ(b.data(), a.data());
}

TEST(SpanTest, CopyConstructor)
{
  vector<int> vec{1, 2, 3, 4, 5};

  {
    Span a = Span<int>(vec.data(), vec.size());

    Span<volatile int> b{a};

    EXPECT_EQ(a.data(), vec.data());
    EXPECT_EQ(a.data(), b.data());

    EXPECT_EQ(a.size(), b.size());
  }
}

TEST(SpanTest, ConstructorCArray)
{
  int tmp[] = {1, 2, 3, 4};

  {
    Span<int> a = tmp;

    EXPECT_EQ(a.data(), tmp);
    EXPECT_EQ(a.size(), size(tmp));
  }
}

TEST(SpanTest, ConstructorStdArray)
{
  array<int, 4> tmp{1, 2, 3, 4};
  {
    Span<int> a = tmp;

    EXPECT_EQ(a.data(), tmp.data());
    EXPECT_EQ(a.size(), size(tmp));
  }
}

TEST(SpanTest, Empty)
{
  {
    Span<int> a{};
    EXPECT_TRUE(a.is_empty());
  }

  {
    Span<int> a(nullptr, 0);
    EXPECT_TRUE(a.is_empty());
  }
}

TEST(SpanTest, At)
{
  int tmp[] = {1, 2, 3, 4};
  {
    Span<int> a(tmp, size(tmp));

    EXPECT_EQ(a.at(4), None);
    EXPECT_EQ(a.at(3), Some(4));
  }
}

TEST(SpanTest, As)
{
  {
    int32_t                tmp[] = {1, 2, 3, 4};
    Span<int32_t>          a(tmp);
    Span<int32_t const>    tmp_a = a.as_const();
    Span<char const>       tmp_b = a.as_char();
    Span<uint8_t const>    tmp_c = a.as_u8();
    Span<volatile int32_t> tmp_d = a.as_volatile();

    (void) tmp_a;
    (void) tmp_b;
    (void) tmp_c;
    (void) tmp_d;

    Span<char const volatile> b = a.as_u8().as_volatile().as_const().as_char();

    EXPECT_EQ((void *) a.data(), (void *) b.data());
    EXPECT_EQ(a.size_bytes(), b.size());
    EXPECT_EQ(a.size() * 4, b.size());
  }
}

struct result_t
{
  size_t size;
  int    value;
};

namespace t1
{
constexpr result_t Span_Slice(size_t index)
{
  int tmp[] = {1, 2, 3, 4};

  Span<int> h = tmp;

  return {h.slice(index).size(), h.slice(index)[0]};
}

}        // namespace t1

TEST(SpanTest, Slice)
{
  using namespace t1;
  {
    constexpr auto a = Span_Slice(0);
    EXPECT_EQ(a.size, 4);
    EXPECT_EQ(a.value, 1);
  }

  {
    constexpr auto a = Span_Slice(3);
    EXPECT_EQ(a.size, 1);
    EXPECT_EQ(a.value, 4);
  }
}

TEST(SpanTest, Algorithms)
{
  int y[] = {1, 2, 3, 4, 5, 6};

  Span<int> r{y};
  Span<int> e{};

  r.fill(8);

  for (auto &u : r)
  {
    std::cout << u << std::endl;
  }

  EXPECT_TRUE(r.all_equals(8));
  EXPECT_FALSE(r.none_equals(8));

  EXPECT_TRUE(e.is_empty());
  EXPECT_FALSE(r.all_equals(0));
  EXPECT_FALSE(r.any_equals(0));
  EXPECT_TRUE(r.none_equals(0));

  EXPECT_TRUE(r.map([](int a) { return a + 1; }, r).all_equals(9));
  r.find(9).fill(64);
  EXPECT_EQ(r[0], 64);

  EXPECT_TRUE(r.contains(9));
  EXPECT_FALSE(r.contains(20));

  for (int &element : r.slice(1))
  {
    EXPECT_EQ(element, 9);
  }

  {
    int g[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

    Span<int> f = g;

    auto [a, b] = f.partition([](int x) { return x < 5; });

    EXPECT_EQ(a.size(), 4);
    EXPECT_EQ(b.size(), 6);

    auto [c, d] = b.partition([](int x) { return x > 8; });

    EXPECT_EQ(c.size(), 2);
    EXPECT_EQ(d.size(), 4);
  }
}

TEST(SpanTest, Last)
{
  int data[] = {1, 2, 3, 4, 5};
  EXPECT_EQ(*stx::Span{data}.last().unwrap(), 5);
  EXPECT_EQ(stx::Span<int>{}.last(), stx::None);
}

TEST(SpanTest, Transmute)
{
  uint16_t data[] = {1, 2, 3, 4, 5};
  EXPECT_EQ((stx::Span{data}.transmute<uint8_t>().size()), 10);
  EXPECT_EQ(((uint16_t *) (stx::Span{data}.transmute<uint8_t>().data())), data);
}

TEST(SpanTest, ReverseOdd)
{
  int data[] = {1, 2, 3, 4, 5};
  stx::Span{data}.reverse();
  EXPECT_EQ(data[0], 5);
  EXPECT_EQ(data[1], 4);
  EXPECT_EQ(data[2], 3);
  EXPECT_EQ(data[3], 2);
  EXPECT_EQ(data[4], 1);
}

TEST(SpanTest, ReverseEven)
{
  int data[] = {1, 2, 3, 4, 5, 6};
  stx::Span{data}.reverse();
  EXPECT_EQ(data[0], 6);
  EXPECT_EQ(data[1], 5);
  EXPECT_EQ(data[2], 4);
  EXPECT_EQ(data[3], 3);
  EXPECT_EQ(data[4], 2);
  EXPECT_EQ(data[5], 1);
}
