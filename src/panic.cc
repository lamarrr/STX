/**
 * @file panic.cc
 * @author Basit Ayantunde <rlamarrr@gmail.com>
 * @brief
 * @version  0.1
 * @date 2020-04-26
 *
 * @copyright Copyright (c) 2020
 *
 */

#include "stx/panic.h"

#include "stx/panic/handlers/default/default.h"

#ifndef STX_OVERRIDE_PANIC_HANDLER

STX_LOCAL void stx::panic_handler(std::string_view info,
                                  ReportPayload const& payload,
                                  SourceLocation location) noexcept {
  panic_default(std::move(info), payload, std::move(location));
}

#endif
