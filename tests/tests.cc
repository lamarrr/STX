/**
 * @file tests.cc
 * @author Basit Ayantunde <rlamarrr@gmail.com>
 * @brief
 * @version  0.1
 * @date 2020-04-18
 *
 * @copyright Copyright (c) 2020
 *
 */
#include "gtest/gtest.h"
#include "stx/panic.h"

/*******************************

Don't just test with simple integral/POD types, also test with complex objects
involving memory allocation.

*******************************/

int main(int argc, char** argv) {
  testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}
