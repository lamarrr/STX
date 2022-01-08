/**
 * @file abort.h
 * @author Basit Ayantunde <rlamarrr@gmail.com>
 * @date 2020-04-26
 *
 * @copyright MIT License
 *
 * Copyright (c) 2020-2021 Basit Ayantunde
 *
 */

#pragma once

#include <cstdlib>

#include "stx/panic.h"

STX_BEGIN_NAMESPACE

    /// Causes the abort instruction to be executed.
    [[noreturn]] inline void
    panic_abort(std::string_view const& info, ReportPayload const& payload,
                SourceLocation const& location) {
  (void)info;
  (void)payload;
  (void)location;

  std::abort();
}

STX_END_NAMESPACE
