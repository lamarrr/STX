/**
 * @file panic.h
 * @author Basit Ayantunde <rlamarrr@gmail.com>
 * @date 2020-04-26
 *
 * @copyright MIT License
 *
 * Copyright (c) 2020-2021 Basit Ayantunde
 *
 */

#pragma once

#include "stx/panic/report.h"
#include "stx/source_location.h"

STX_BEGIN_NAMESPACE

/// The global panic handler.
/// It is advisable to be avoid heap memory allocation of any sort and be
/// conscious of shared state as it can be called from mulitple threads.
///
void panic_handler(std::string_view info, ReportPayload const& payload,
                   SourceLocation location) noexcept;

/// Handles and dispatches the panic handler. The debugging breakpoint should be
/// attached to this function to investigate panics.
///
/// # WARNING
///
/// DO NOT INVOKE THIS FUNCTION!!!
///
[[noreturn]] STX_DLL_EXPORT void begin_panic(std::string_view info,
                                             ReportPayload const& payload,
                                             SourceLocation location) noexcept;

/// This allows a program to terminate immediately and provide feedback to the
/// caller of the program. `panic` should be used when a program reaches an
/// unrecoverable state. This function is the perfect way to assert conditions
/// in example code and in tests. `panic` is closely tied with the `unwrap` and
/// `expect` method of both `Option` and `Result`. Both implementations call
/// `panic` when they are set to `None` or `Err` variants.
///
template <typename T>
[[noreturn]] STX_FORCE_INLINE void panic(
    std::string_view info, T const& value,
    SourceLocation location = SourceLocation::current()) noexcept {
  begin_panic(info, ReportPayload(report_query >> value), location);
}

template <typename T = void>
[[noreturn]] STX_FORCE_INLINE void panic(
    std::string_view info = "explicit panic",
    SourceLocation location = SourceLocation::current()) noexcept {
  begin_panic(info, ReportPayload(SpanReport()), location);
}

STX_END_NAMESPACE
