/**
 * @file panic_test.cc
 * @author Basit Ayantunde <rlamarrr@gmail.com>
 * @brief
 * @version  0.1
 * @date 2020-04-16
 *
 * @copyright Copyright (c) 2020
 *
 */

#include "stx/panic.h"

#include <iostream>

#include "gtest/gtest.h"
#include "handler/throw.h"

using namespace stx;

TEST(PanicTest, PanicThrow)
try {
  panic_throw("Panic test message");
} catch (PanicInfo const& panic) {
  auto line = panic.location().line();
  auto column = panic.location().column();
  auto file = panic.location().file_name();
  auto function_name = panic.location().function_name();
  auto info = panic.info();

  EXPECT_TRUE((line == 0) || (line == __LINE__ - 8UL));
  EXPECT_TRUE((column == 0) || (column > 2 && column < 38));
  EXPECT_TRUE((file == nullptr) ||
              (std::string(file) == std::string(__FILE__)));
  EXPECT_TRUE((function_name == nullptr) ||
              (function_name == std::string("TestBody")));
  EXPECT_EQ(info, std::string("Panic test message"));
}
