
#include "stx/string.h"

#include "gtest/gtest.h"

using namespace stx;

TEST(StrTest, Init) {
  String str;

  String a{stx::string::make_static("hello")};
  String b{stx::string::make(os_allocator, "waddup").unwrap()};
  //
}
