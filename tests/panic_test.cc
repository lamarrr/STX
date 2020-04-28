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
#include "stx/panic/handlers/throw/throw.h"

using namespace stx;

#define LOG(detail, x)                                                    \
  std::cout << "[" << __FILE__ << ":" << std::to_string(__LINE__) << "] " \
            << detail << ": " << x << std::endl

TEST(PanicTest, PanicThrow)
try {
  panic_throw("Panic test message");
} catch (PanicInfo const& panic) {
  EXPECT_EQ(panic.location().line(), __LINE__ - 2UL);
  EXPECT_EQ(panic.location().file_name(), __FILE__);
  EXPECT_EQ(panic.info(), "Panic test message");
  LOG("Panic Column", panic.location().column());
  LOG("Panic Function Name", panic.location().function_name());
  EXPECT_EQ(panic.location().function_name(), "TestBody");
}
