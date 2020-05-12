#pragma once

#include <cstdio>      // i/o
#include <functional>  // hash
#include <mutex>       // mutex
#include <thread>      // thread id

#include "stx/panic.h"

namespace stx {

// probably too much, but enough
// this will at least hold a formatted uint128_t (40 digits)

namespace internal {
namespace panic_util {
constexpr int kFormatBufferSize = 256;
constexpr auto kThreadIdHash = std::hash<std::thread::id>{};
};  // namespace panic_util
};  // namespace internal

// this should be made re-entrant
[[noreturn]] inline void panic_default(
    std::string_view info, ReportPayload const& payload,
    SourceLocation location = SourceLocation::current()) noexcept {
  using namespace internal::panic_util;
  static std::mutex stderr_lock;

  char log_buffer[kFormatBufferSize];

  auto thread_id_hash = kThreadIdHash(std::this_thread::get_id());

  stderr_lock.lock();

  std::fputs("thread with hash: '", stderr);

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

  // threads will still be able to log for some nanoseconds
  stderr_lock.unlock();

  std::abort();
}
}  // namespace stx
