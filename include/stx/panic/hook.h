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

#include <atomic>

#include "stx/panic.h"

//! @file
//!
//! Hooks are useful for writing device drivers where you load the drivers at
//! runtime as a DLL. As the default behaviour is to log to stderr and abort, In
//! drivers and safety-critical software, You often don't want a driver failure
//! to cause the whole program to immediately cause the whole program to abort
//! due to a panic from the drivers. Hooks allow you to control the panic
//! behaviour at runtime.
//!
//!
//! # NOTE
//!
//! You might have to do some extra demangling as it is not exposed via a
//! C ABI for consistency and internal reasons.
//!
//! Process:
//! - Check if hooks are available for attaching: `has_panic_hook()`
//! - If hooks are available, attach a panic hook:
//! `attach_panic_hook(my_handler)` or reset the exisiting panic hook back to
//! the default: `take_panic_hook()`
//!
//!

namespace stx {

#if defined(STX_VISIBLE_PANIC_HOOK)
constexpr bool kHasPanicHook = true;
#else
constexpr bool kHasPanicHook = false;
#endif

// multiple threads can try to modify/read the hook at once.
using PanicHook = decltype(panic_handler)*;
using AtomicPanicHook = std::atomic<PanicHook>;

namespace this_thread {

/// # THREAD-SAFETY
/// thread-safe.
STX_EXPORT bool is_panicking() noexcept;
};  // namespace this_thread

/// Checks if panic hooks are enabled and visible.
/// This should be called before calling any of `attach_panic_hook` or
/// `take_panic_hook`.
///
/// # THREAD-SAFETY
///
/// thread-safe.
STX_EXPORT bool has_panic_hook() noexcept;

#if defined(STX_VISIBLE_PANIC_HOOK)

/// Attaches a new panic hook, the attached panic hook is called in place of the
/// default panic hook.
///
/// # THREAD-SAFETY
///
/// thread-safe.
STX_EXPORT bool attach_panic_hook(PanicHook hook) noexcept;

/// Removes the registered panic hook (if any) and let's it resort to the
/// default.
///
/// # THREAD-SAFETY
///
/// thread-safe.
STX_EXPORT bool take_panic_hook(PanicHook* hook) noexcept;

#endif

};  // namespace stx
