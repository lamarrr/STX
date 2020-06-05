/**
 * @file panic.h
 * @author Basit Ayantunde <rlamarrr@gmail.com>
 * @version  0.0.1
 * @date 2020-04-26
 *
 * @copyright MIT License
 *
 * Copyright (c) 2020 Basit Ayantunde
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
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
    std::string_view info = "explicit panic",
    SourceLocation location = SourceLocation::current()) noexcept {
  begin_panic(std::move(info), ReportPayload(Report("")), std::move(location));
}

}  // namespace stx
