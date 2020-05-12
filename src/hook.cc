#include "stx/panic/hook.h"

bool stx::has_panic_hook() noexcept { return kHasPanicHook; }

#ifdef STX_ENABLE_PANIC_HOOK
namespace stx {
namespace this_thread {
namespace internal {
/// increases the panic count for this thread by `step`
STX_FORCE_INLINE size_t step_panic_count(size_t step) noexcept {
  thread_local static size_t panic_count = 0;
  panic_count += step;
  return panic_count;
}
}  // namespace internal
}  // namespace this_thread

namespace internal {
STX_FORCE_INLINE AtomicPanicHook& panic_hook_ref() noexcept {
  static AtomicPanicHook hook{nullptr};
  return hook;
}
}  // namespace internal
}  // namespace stx

// safety?
// do we need to separate them into atomic reads and writes?
bool stx::this_thread::is_panicking() noexcept {
  return stx::this_thread::internal::step_panic_count(0) != 0;
}

void stx::attach_panic_hook(stx::PanicHook hook) noexcept {
  if (this_thread::is_panicking()) std::abort();
  stx::internal::panic_hook_ref().exchange(hook, std::memory_order::seq_cst);
}

stx::PanicHook stx::take_panic_hook() noexcept {
  if (stx::this_thread::is_panicking()) std::abort();
  return stx::internal::panic_hook_ref().exchange(nullptr,
                                                  std::memory_order::seq_cst);
}

void stx::default_panic_hook(std::string_view, ReportPayload const&,
                             SourceLocation) noexcept {}

#endif
