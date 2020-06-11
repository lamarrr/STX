/**
 * @file panic.h
 * @author Basit Ayantunde <rlamarrr@gmail.com>
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

STX_BEGIN_NAMESPACE

/// The global panic handler.
/// It is advisable to be avoid heap memory allocation of any sort and be
/// conscious of shared state as it can be called from mulitple threads.
///
void panic_handler(std::string_view const& info, ReportPayload const& payload,
                   SourceLocation const& location) noexcept;

/// Handles and dispatches the panic handler. The debugging breakpoint should be
/// attached to this function to investigate panics.
///
/// # WARNING
///
/// DO NOT INVOKE THIS FUNCTION!!!
///
[[noreturn]] void begin_panic(std::string_view const& info,
                              ReportPayload const& payload,
                              SourceLocation const& location) noexcept;

/// This allows a program to terminate immediately and provide feedback to the
/// caller of the program. `panic` should be used when a program reaches an
/// unrecoverable state. This function is the perfect way to assert conditions
/// in example code and in tests. `panic` is closely tied with the `unwrap` and
/// `expect` method of both `Option` and `Result`. Both implementations call
/// `panic` when they are set to `None` or `Err` variants.
///
template <typename T>
[[noreturn]] STX_FORCE_INLINE void panic(
    std::string_view const& info, T const& value,
    SourceLocation const& location = SourceLocation::current()) noexcept {
  begin_panic(info, ReportPayload(report_query >> value), location);
}

template <typename T = void>
[[noreturn]] STX_FORCE_INLINE void panic(
    std::string_view const& info = "explicit panic",
    SourceLocation const& location = SourceLocation::current()) noexcept {
  begin_panic(info, ReportPayload(SpanReport()), location);
}

STX_END_NAMESPACE
