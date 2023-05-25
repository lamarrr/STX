#include "stx/string.h"

#include <string>
#include <vector>

#include "gtest/gtest.h"

using namespace stx;

TEST(StrTest, Init)
{
  String str;

  String a = "hello"_str;
  String b{string::make(os_allocator, "waddup").unwrap()};

  EXPECT_EQ(str.data()[0], '\0');
  EXPECT_EQ(str.size(), 0);

  EXPECT_EQ(a, "hello");
  EXPECT_EQ(b, "waddup");

  EXPECT_EQ(
      string::join(os_allocator, " ", "Hello,", "Beautiful", "World!").unwrap(),
      "Hello, Beautiful World!");

  EXPECT_EQ(string::join(os_allocator, "???", "Hello,", "World!").unwrap(),
            "Hello,???World!");

  EXPECT_EQ(string::join(os_allocator, "???", "", "", "").unwrap(),
            string::make_static("??????"));

  EXPECT_EQ(string::join(os_allocator, "", "", "", "").unwrap(),
            string::make_static(""));

  EXPECT_EQ(string::upper(os_allocator, "Hello, World!").unwrap(),
            "HELLO, WORLD!");
  EXPECT_EQ(string::lower(os_allocator, "hello, world!").unwrap(),
            "hello, world!");

  String views[] = {"Hello,", stx::string::make_static("Beautiful"), stx::string::make_static("World!")};

  Span x = views;
  EXPECT_EQ(string::join(os_allocator, " ", x).unwrap(),
            "Hello, Beautiful World!");

  EXPECT_EQ(a, a.copy(stx::os_allocator).unwrap());
}
