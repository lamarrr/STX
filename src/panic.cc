/**
 * @file panic.cc
 * @author Basit Ayantunde <rlamarrr@gmail.com>
 * @date 2020-04-26
 *
 * @copyright MIT License
 *
 * Copyright (c) 2020-2022 Basit Ayantunde
 *
 */

#include "stx/panic.h"

#ifndef STX_CUSTOM_PANIC_HANDLER

#include "stx/panic/default.h"

void stx::panic_handler(std::string_view info, std::string_view error_report,
                        stx::SourceLocation location) {
  stx::panic_default(info, error_report, location);
}

#endif
