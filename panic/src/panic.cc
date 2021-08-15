/**
 * @file panic.cc
 * @author Basit Ayantunde <rlamarrr@gmail.com>
 * @date 2020-04-26
 *
 * @copyright MIT License
 *
 * Copyright (c) 2020-2021 Basit Ayantunde
 *
 */

#include "stx/panic.h"

#ifndef STX_CUSTOM_PANIC_HANDLER

#include "stx/panic/default.h"

// TODO(lamarrr): make weak
void stx::panic_handler(std::string_view info,
                        stx::ReportPayload const& payload,
                        stx::SourceLocation location) noexcept {
  stx::panic_default(info, payload, location);
}

#endif

