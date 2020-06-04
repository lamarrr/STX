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
constexpr bool kVisiblePanicHook = true;
#else
constexpr bool kVisiblePanicHook = false;
#endif

// multiple threads can try to modify/read the hook at once.
using PanicHook = decltype(panic_handler)*;
using AtomicPanicHook = std::atomic<PanicHook>;

namespace this_thread {

/// Checks if the current thread is panicking.
///
/// # THREAD-SAFETY
/// thread-safe.
[[nodiscard]] STX_EXPORT bool is_panicking() noexcept;
}  // namespace this_thread

/// Checks if panic hooks are visible to be attached-to when loaded as a dynamic
/// library. This should be called before calling any of `attach_panic_hook` or
/// `take_panic_hook` when loaded as a dynamic library.
///
/// # THREAD-SAFETY
///
/// thread-safe.
[[nodiscard]] STX_EXPORT bool panic_hook_visible() noexcept;

/// Attaches a new panic hook, the attached panic hook is called in place of the
/// default panic hook.
///
/// Returns `true` if the thread is not panicking and the panic hook was
/// successfully attached, else returns `false`.
///
/// # THREAD-SAFETY
///
/// thread-safe.

[[nodiscard]]
#if defined(STX_VISIBLE_PANIC_HOOK)
STX_EXPORT
#else
STX_LOCAL
#endif

    bool
    attach_panic_hook(PanicHook hook) noexcept;

/// Removes the registered panic hook (if any) and resets it to the
/// default panic hook.
/// `hook` is set to the last-registered panic hook or the default.
///
/// Returns `true` if the thread is not panicking and the panic hook was
/// successfully taken, else returns `false`.
///
/// # THREAD-SAFETY
///
/// thread-safe.

[[nodiscard]]
#if defined(STX_VISIBLE_PANIC_HOOK)
STX_EXPORT
#else
STX_LOCAL
#endif

    bool
    take_panic_hook(PanicHook* hook) noexcept;

}  // namespace stx
