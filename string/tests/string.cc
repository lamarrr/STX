#include "stx/string.h"

#include <string>
#include <vector>

#include "gtest/gtest.h"

using namespace stx;

TEST(StrTest, Init) {
  // TODO(lamarrr): add more tests

  String str;

  String a{string::make_static("hello")};
  String b{string::make(os_allocator, "waddup").unwrap()};

  EXPECT_EQ(str[str.size()], '\0');

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

  {
    std::vector<String> vec;
    string::split("a,b,c,d,e,f", ",", [&](StringView token) {
      vec.push_back(string::make(os_allocator, token).unwrap());
    });
  }

  {
    std::vector<String> vec;
    string::split("a,b,c,d,e,f,", ",", [&](StringView token) {
      vec.push_back(string::make(os_allocator, token).unwrap());
    });
  }

  EXPECT_EQ(string::upper(os_allocator, "Hello, World!").unwrap(),
            "HELLO, WORLD!");
  EXPECT_EQ(string::lower(os_allocator, "hello, world!").unwrap(),
            "hello, world!");

  std::vector<std::string> views{
      std::string{"Hello,"}, std::string{"Beautiful"}, std::string{"World!"}};

Span x = views;
  EXPECT_EQ(string::join_span(os_allocator, " ", x).unwrap(),
            "Hello, Beautiful World!");
}
