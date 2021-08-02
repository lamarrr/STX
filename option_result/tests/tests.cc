/**
 * @file tests.cc
 * @author Basit Ayantunde <rlamarrr@gmail.com>
 * @date 2020-04-18
 *
 * @copyright MIT License
 *
 * Copyright (c) 2020-2021 Basit Ayantunde
 *
 */

#include "gtest/gtest.h"

/*******************************

Don't just test with simple integral/POD/trivial types, also test with complex
objects involving memory allocation.

*******************************/

int main(int argc, char** argv) {
  testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}
