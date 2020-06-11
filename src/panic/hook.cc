/**
 * @file hook.cc
 * @author Basit Ayantunde <rlamarrr@gmail.com>
 * @date 2020-05-16
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

#include "stx/panic/hook.h"

#include <cstdlib>

STX_BEGIN_NAMESPACE

namespace this_thread {
namespace {

/// increases the panic count for this thread by `step`
STX_LOCAL size_t step_panic_count(size_t step) noexcept {
  thread_local static size_t panic_count = 0;
  panic_count += step;
  return panic_count;
}
}  // namespace
}  // namespace this_thread

STX_LOCAL AtomicPanicHook& panic_hook_ref() noexcept {
  static AtomicPanicHook hook{nullptr};
  return hook;
}

STX_EXPORT bool panic_hook_visible() noexcept { return kPanicHookVisible; }

STX_EXPORT bool this_thread::is_panicking() noexcept {
  return this_thread::step_panic_count(0) != 0;
}

// the panic hook takes higher precedence over the panic handler
STX_LOCAL void default_panic_hook(std::string_view const& info,
                                  ReportPayload const& payload,
                                  SourceLocation const& location) noexcept {
  panic_handler(info, payload, location);
}

#if defined(STX_VISIBLE_PANIC_HOOK)
STX_EXPORT
#else
STX_LOCAL
#endif

bool attach_panic_hook(PanicHook hook) noexcept {
  if (this_thread::is_panicking()) return false;
  panic_hook_ref().exchange(hook, std::memory_order_seq_cst);
  return true;
}

#if defined(STX_VISIBLE_PANIC_HOOK)
STX_EXPORT
#else
STX_LOCAL
#endif

bool take_panic_hook(PanicHook* out) noexcept {
  if (this_thread::is_panicking()) return false;
  auto hook = panic_hook_ref().exchange(nullptr, std::memory_order_seq_cst);
  if (hook == nullptr) {
    *out = default_panic_hook;
  } else {
    *out = hook;
  }
  return true;
}

[[noreturn]] void begin_panic(std::string_view const& info,
                              ReportPayload const& payload,
                              SourceLocation const& location) noexcept {
  // TODO(lamarrr): We probably need a method for stack unwinding, So we can
  // free held resources

  // detecting recursive panics
  if (this_thread::step_panic_count(1) > 1) {
    std::fputs("thread panicked while processing a panic. aborting...\n",
               stderr);
    std::fflush(stderr);
    std::abort();
  }

  // all threads use the same panic hook
  PanicHook hook = panic_hook_ref().load(std::memory_order_seq_cst);

  if (hook != nullptr) {
    hook(info, payload, location);
  } else {
    default_panic_hook(info, payload, location);
  }

  std::abort();
}

STX_END_NAMESPACE
