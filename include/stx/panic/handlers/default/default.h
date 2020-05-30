/**
 * @file default.h
 * @author Basit Ayantunde <rlamarrr@gmail.com>
 * @brief
 * @version  0.1
 * @date 2020-05-22
 *
 * @copyright Copyright (c) 2020
 *
 */

#pragma once

#include <mutex>   // mutex NOLINT
#include <thread>  // thread::id NOLINT

#if defined(STX_ENABLE_PANIC_BACKTRACE)
#include "stx/backtrace.h"
#endif

namespace stx {

// probably too much, but enough
// this will at least hold a formatted uint128_t (40 digits)

namespace internal {
namespace panic_util {
constexpr int kFormatBufferSize = 256;
constexpr auto kThreadIdHash = std::hash<std::thread::id>{};
};  // namespace panic_util
};  // namespace internal

// this should be made thread-safe.
inline void panic_default(
    std::string_view info, ReportPayload const& payload,
    SourceLocation location = SourceLocation::current()) noexcept {
  using namespace internal::panic_util;  // NOLINT

  static std::mutex stderr_lock;

  char log_buffer[kFormatBufferSize];

  auto thread_id_hash = kThreadIdHash(std::this_thread::get_id());

  stderr_lock.lock();

  std::fputs("\nthread with hash: '", stderr);

  std::snprintf(log_buffer, kFormatBufferSize, "%zu", thread_id_hash);
  std::fputs(log_buffer, stderr);

  std::fputs("' panicked with: '", stderr);

  for (char c : info) {
    std::fputc(c, stderr);
  }

  if (!payload.data().empty()) {
    std::fputc(':', stderr);
    std::fputc(' ', stderr);

    for (auto c : payload.data()) {
      std::fputc(c, stderr);
    }
  }

  std::fputc('\'', stderr);

  std::fputs(" at function: '", stderr);

  if (location.function_name() != nullptr) {
    std::fputs(location.function_name(), stderr);
  } else {
    std::fputs("<unknown>", stderr);
  }

  std::fputs("' [", stderr);

  if (location.file_name() != nullptr) {
    std::fputs(location.file_name(), stderr);
  } else {
    std::fputs("<unknown>", stderr);
  }

  std::fputc(':', stderr);

  if (location.line() != 0) {
    std::snprintf(log_buffer, kFormatBufferSize, "%d", location.line());
    std::fputs(log_buffer, stderr);
  } else {
    std::fputs("<unknown>", stderr);
  }

  std::fputc(':', stderr);

  if (location.column() != 0) {
    std::snprintf(log_buffer, kFormatBufferSize, "%d", location.column());
    std::fputs(log_buffer, stderr);
  } else {
    std::fputs("<unknown>", stderr);
  }

  std::fputs("]\n", stderr);

  std::fflush(stderr);

#if defined(STX_ENABLE_PANIC_BACKTRACE)
  // assumes the presence of an operating system

  std::fputs(
      "\nBacktrace:\nip: Instruction Pointer,  sp: Stack "
      "Pointer\n\n",
      stderr);

  stx::backtrace::trace([](backtrace::Frame frame, int i) {
    auto const print_none = []() { std::fputs("<unknown>", stderr); };
    auto const print_ptr = [](Ref<uintptr_t> ip) {
      std::fprintf(stderr, "0x%" PRIxPTR, ip.get());
    };

    std::fprintf(stderr, "#%d\t\t", i);

    frame.symbol.as_ref().match(
        [](Ref<backtrace::Symbol> sym) {
          for (char c : sym.get().raw()) {
            std::fputc(c, stderr);
          }
        },
        print_none);

    std::fputs("\t (ip: ", stderr);

    frame.ip.as_ref().match(print_ptr, print_none);

    std::fputs(", sp: ", stderr);

    frame.sp.as_ref().match(print_ptr, print_none);

    std::fputs(")\n", stderr);

    return false;
  });

  std::fputs("\n", stderr);

#endif

  // other threads will still be able to log for some nanoseconds
  stderr_lock.unlock();
}
}  // namespace stx
