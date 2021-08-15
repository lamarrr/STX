/**
 * @file halt.h
 * @author Basit Ayantunde <rlamarrr@gmail.com>
 * @date 2020-04-26
 *
 * @copyright MIT License
 *
 * Copyright (c) 2020-2021 Basit Ayantunde
 *
 */

#pragma once

#include "stx/panic.h"

STX_BEGIN_NAMESPACE

/// Causes the program, or the current thread, to halt by entering an infinite
/// loop.
/// You can force the program to continue via a debugger by setting the
/// `halt` value to true. after which the program will run cleanup code and
/// abort.
inline void panic_halt(std::string_view const& info,
                       ReportPayload const& payload,
                       SourceLocation const& location) {
  (void)info;
  (void)payload;
  (void)location;

  volatile bool const halt = true;
  while (halt) {
  }
}

STX_END_NAMESPACE
