
#include "stx/string.h"

#include "gtest/gtest.h"

using namespace stx;

TEST(StrTest, Init) {
  String str;

  String a{stx::string::make_static("hello")};
  String b{stx::string::make(os_allocator, "waddup").unwrap()};

  EXPECT_EQ(str[str.size()], '\0');

  EXPECT_EQ(
      stx::string::join(stx::os_allocator, " ", "Hello,", "Beautiful", "World!")
          .unwrap(),
      stx::string::make_static("Hello, Beautiful World!"));

  EXPECT_EQ(
      stx::string::join(stx::os_allocator, "???", "Hello,", "World!").unwrap(),
      stx::string::make_static("Hello,???World!"));

  EXPECT_EQ(stx::string::join(stx::os_allocator, "???", "", "", "").unwrap(),
            stx::string::make_static("??????"));
  // TODO(lamarrr): add more tests
}
