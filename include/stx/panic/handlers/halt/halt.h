/**
 * @file halt.h
 * @author Basit Ayantunde <rlamarrr@gmail.com>
 * @brief
 * @version  0.1
 * @date 2020-04-26
 *
 * @copyright Copyright (c) 2020
 *
 */

#pragma once

#include "stx/panic.h"

namespace stx {

/// Causes the program, or the current thread, to halt by entering an infinite
/// loop.
/// You can force the program to continue via a debugger by setting the
/// `proceed` value to true.
inline void panic_halt(
    std::string_view info, ReportPayload const& payload,
    SourceLocation location = SourceLocation::current()) noexcept {
  (void)info;
  (void)payload;
  (void)location;

  volatile bool const halt = true;
  while (halt) {
  }
}
};  // namespace stx
