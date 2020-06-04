/**
 * @file panic.h
 * @author Basit Ayantunde <rlamarrr@gmail.com>
 * @brief
 * @version  0.1
 * @date 2020-04-26
 *
 * @copyright Copyright (c) 2020
 *
 */

#pragma once

#include "stx/report.h"
#include "stx/source_location.h"

namespace stx {

// here, we can avoid any form of memory allocation that might be needed,
// therefore deferring the info string and report payload to the callee and can
// also use a stack allocated string especially in cases where dynamic memory
// allocation is undesired
STX_LOCAL void panic_handler(std::string_view info,
                             ReportPayload const& payload,
                             SourceLocation location) noexcept;

/// Handles and dispatches the panic handler. The debugging breakpoint should be
/// attached to this function to investigate panics.
[[noreturn]] STX_LOCAL void begin_panic(std::string_view info,
                                        ReportPayload const& payload,
                                        SourceLocation location) noexcept;

/// This allows a program to terminate immediately and provide feedback to the
/// caller of the program. `panic` should be used when a program reaches an
/// unrecoverable state. This function is the perfect way to assert conditions
/// in example code and in tests. `panic` is closely tied with the `unwrap` and
/// `expect` method of both `Option` and `Result`. Both implementations call
/// `panic` when they are set to `None` or `Err` variants.

template <typename T>
[[noreturn]] STX_FORCE_INLINE void panic(
    std::string_view info, T const& value,
    SourceLocation location = SourceLocation::current()) noexcept {
  begin_panic(std::move(info), ReportPayload(internal::report::query >> value),
              std::move(location));
}

template <typename T = void>
[[noreturn]] STX_FORCE_INLINE void panic(
    std::string_view info,
    SourceLocation location = SourceLocation::current()) noexcept {
  begin_panic(std::move(info), ReportPayload(Report("")), std::move(location));
}

};  // namespace stx
