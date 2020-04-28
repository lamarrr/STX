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

#include "stx/panic/handlers/throw/throw.h"

namespace stx {
#ifndef STX_OVERRIDE_PANIC_HANDLER

[[noreturn]] void panic_handler(std::string_view info,
                                SourceLocation location) {
  panic_throw(std::move(info), std::move(location));
}

#endif
};  // namespace stx
