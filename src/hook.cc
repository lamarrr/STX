/**
 * @file hook.cc
 * @author Basit Ayantunde <rlamarrr@gmail.com>
 * @date 2020-05-16
 *
 * @copyright MIT License
 *
 * Copyright (c) 2020-2022 Basit Ayantunde
 *
 */

#include "stx/panic/hook.h"

#include <cstdlib>

STX_BEGIN_NAMESPACE

namespace this_thread
{
namespace
{

/// increases the panic count for this thread by `step`
size_t step_panic_count(size_t step)
{
  thread_local static size_t panic_count = 0;
  panic_count += step;
  return panic_count;
}
}        // namespace
}        // namespace this_thread

STX_DLL_EXPORT AtomicPanicHook &panic_hook_ref()
{
  static AtomicPanicHook hook{nullptr};
  return hook;
}

STX_DLL_EXPORT bool this_thread::is_panicking()
{
  return this_thread::step_panic_count(0) != 0;
}

// the panic hook takes higher precedence over the panic handler
void default_panic_hook(std::string_view info, std::string_view error_report,
                        SourceLocation location)
{
  panic_handler(info, error_report, location);
}

STX_DLL_EXPORT bool attach_panic_hook(PanicHook hook)
{
  if (this_thread::is_panicking())
    return false;
  panic_hook_ref().exchange(hook, std::memory_order_seq_cst);
  return true;
}

STX_DLL_EXPORT bool take_panic_hook(PanicHook *out)
{
  if (this_thread::is_panicking())
    return false;
  auto hook = panic_hook_ref().exchange(nullptr, std::memory_order_seq_cst);
  if (hook == nullptr)
  {
    *out = default_panic_hook;
  }
  else
  {
    *out = hook;
  }
  return true;
}

STX_END_NAMESPACE

[[noreturn]] STX_DLL_EXPORT void stx::begin_panic(
    std::string_view info, std::string_view error_report,
    stx::SourceLocation location)
{
  // detecting recursive panics
  if (this_thread::step_panic_count(1) > 1)
  {
    std::fputs("thread panicked while processing a panic. aborting...\n",
               stderr);
    std::fflush(stderr);
    std::abort();
  }

  // all threads use the same panic hook
  PanicHook hook = stx::panic_hook_ref().load(std::memory_order_seq_cst);

  if (hook != nullptr)
  {
    hook(info, error_report, location);
  }
  else
  {
    stx::default_panic_hook(info, error_report, location);
  }

  std::abort();
}
