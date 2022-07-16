/**
 * @file panic_test.cc
 * @author Basit Ayantunde <rlamarrr@gmail.com>
 * @date 2020-04-16
 *
 * @copyright MIT License
 *
 * Copyright (c) 2020-2022 Basit Ayantunde
 *
 */

#include "stx/panic.h"

#include "gtest/gtest.h"

TEST(PanicTest, Panics) {
  EXPECT_DEATH_IF_SUPPORTED(stx::panic(), ".*");
  EXPECT_DEATH_IF_SUPPORTED(stx::panic("hello, world"), ".*");
}
