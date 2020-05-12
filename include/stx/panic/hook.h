/**
 * @file hook.h
 * @author Basit Ayantunde <rlamarrr@gmail.com>
 * @brief
 * @version  0.1
 * @date 2020-05-08
 *
 * @copyright Copyright (c) 2020
 *
 */
#pragma once

#ifdef STX_ENABLE_BACK_TRACE
#ifdef STX_OVERRIDE_PANIC_HANDLER
#error "Panic Handler overriden, STX can thus not provide a panic backtrace"
#else
#include <cxxabi.h>
#include <unwind.h>
#endif
#endif

#include <atomic>
#include <cstdlib>

#include "stx/panic.h"  // separate
#include "stx/report.h"

// turned off by default
// this should be hidden from the library user

#define STX_ENABLE_PANIC_HOOK

namespace stx {

using panic_handler_t = decltype(panic_handler)*;

#ifdef STX_ENABLE_PANIC_HOOK
constexpr bool kHasPanicHook = true;
#else
constexpr bool kHasPanicHook = false;
#endif

// multiple threads can try to modify/read the hook at once.
using PanicHook = panic_handler_t;
using AtomicPanicHook = std::atomic<PanicHook>;

namespace this_thread {
bool is_panicking() noexcept;
};  // namespace this_thread

// ABI
/// Check if panic hooks are enabled.
/// This should be called before calling any of attach_panic_hook or
/// reset_panic_hook
bool has_panic_hook() noexcept;

#ifdef STX_ENABLE_PANIC_HOOK
void attach_panic_hook(PanicHook hook) noexcept;
PanicHook take_panic_hook() noexcept;
#endif

void default_panic_hook(std::string_view, ReportPayload const&,
                        SourceLocation) noexcept;

};  // namespace stx
