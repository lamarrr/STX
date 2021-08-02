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

#ifndef STX_CUSTOM_PANIC_HANDLER

#include "stx/panic.h"

#include "stx/panic/default.h"

STX_BEGIN_NAMESPACE


// TODO(lamarrr): make weak
void panic_handler(std::string_view const& info, ReportPayload const& payload,
                   SourceLocation const& location) noexcept {
  panic_default(info, payload, location);
}

STX_END_NAMESPACE

#endif
