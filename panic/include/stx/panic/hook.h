/**
 * @file hook.h
 * @author Basit Ayantunde <rlamarrr@gmail.com>
 * @date 2020-05-08
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
//! the default: `take_panic_hook(&installed_handler)`
//!
//!

STX_BEGIN_NAMESPACE

#if defined(STX_VISIBLE_PANIC_HOOK)
constexpr bool kPanicHookVisible = true;
#else
constexpr bool kPanicHookVisible = false;
#endif

// multiple threads can try to modify/read the hook at once.
using PanicHook = decltype(panic_handler)*;
using AtomicPanicHook = std::atomic<PanicHook>;

namespace this_thread {

/// Checks if the current thread is panicking.
///
/// # Thread-safe?
///
/// Yes
///
[[nodiscard]] STX_EXPORT bool is_panicking() noexcept;
}  // namespace this_thread

/// Checks if panic hooks are visible to be attached-to when loaded as a dynamic
/// library. This should be called before calling any of `attach_panic_hook` or
/// `take_panic_hook` when loaded as a dynamic library.
///
/// # Thread-safe?
///
/// Yes
///
[[nodiscard]] STX_EXPORT bool panic_hook_visible() noexcept;

/// Attaches a new panic hook, the attached panic hook is called in place of the
/// default panic hook.
///
/// Returns `true` if the thread is not panicking and the panic hook was
/// successfully attached, else returns `false`.
///
/// # Thread-safe?
///
/// Yes
///
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
/// # Thread-safe?
///
/// Yes
///
[[nodiscard]]
#if defined(STX_VISIBLE_PANIC_HOOK)
STX_EXPORT
#else
STX_LOCAL
#endif

    bool
    take_panic_hook(PanicHook* hook) noexcept;

STX_END_NAMESPACE
