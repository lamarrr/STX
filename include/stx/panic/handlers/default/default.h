/**
 * @file default.h
 * @author Basit Ayantunde <rlamarrr@gmail.com>
 * @date 2020-05-22
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

#if !defined(STX_NO_STD_THREAD_MUTEX)
#include <mutex>   // NOLINT
#include <thread>  // NOLINT
#endif

#include "stx/panic.h"
#include "stx/panic/handlers/print.h"

#if defined(STX_ENABLE_PANIC_BACKTRACE)
#include "stx/backtrace.h"
#endif

STX_BEGIN_NAMESPACE

// here, we can avoid any form of memory allocation that might be needed,
// therefore deferring the info string and report payload to the callee and can
// also use a stack allocated string especially in cases where dynamic memory
// allocation is undesired
// this is also thread-safe.
//
// most considerations here are for constrained systems. We can't just use
// `printf` as it is implementation-defined whether it uses dynamic memory
// allocation or not. Also, we can't pack the strings into a buffer and `fputs`
// at once as the buffer can likely not be enough, instead we reuse the buffer.
// May not be fast, but it is a panic anyway.
//
inline void panic_default(std::string_view const& info,
                          ReportPayload const& payload,
                          SourceLocation const& location) noexcept {
  // probably too much, but enough
  // this will at least hold a formatted uint128_t (40 digits)
  static constexpr const int kFmtBufferSize = 64;

#if !defined(STX_NO_STD_THREAD_MUTEX)

  static constexpr const auto kThreadIdHasher = std::hash<std::thread::id>{};

  static std::mutex stderr_lock;

  // we use this buffer for all formatting operations. as it is implementation
  // defined whether fprintf uses dynamic mem alloc
  // only one thread can print at a time so this is safe
  static char fmt_buffer[kFmtBufferSize];

  stderr_lock.lock();

  thread_local size_t const thread_id_hash =
      kThreadIdHasher(std::this_thread::get_id());

#else

  // we can't be too sure that the user won't panic from multiple threads even
  // though they seem to be disabled
  char fmt_buffer[kFmtBufferSize];

#endif

  std::fputs("\nthread", stderr);

#if !defined(STX_NO_STD_THREAD_MUTEX)

  std::fputs(" with hash: '", stderr);

  STX_PANIC_EPRINTF_WITH(fmt_buffer, kFmtBufferSize, "%zu", thread_id_hash);

#endif

  std::fputs("' panicked with: '", stderr);

  for (char c : info) {
    std::fputc(c, stderr);
  }

  if (!payload.data().empty()) {
    std::fputs(": ", stderr);

    for (auto c : payload.data()) {
      std::fputc(c, stderr);
    }
  }

  std::fputs("' at function: '", stderr);

  std::fputs(location.function_name(), stderr);

  std::fputs("' [", stderr);

  std::fputs(location.file_name(), stderr);

  std::fputc(':', stderr);

  auto line = location.line();

  if (line != 0) {
    STX_PANIC_EPRINTF_WITH(fmt_buffer, kFmtBufferSize, "%" PRIuLEAST32, line);
  } else {
    std::fputs("unknown", stderr);
  }

  std::fputc(':', stderr);

  auto column = location.column();

  if (column != 0) {
    STX_PANIC_EPRINTF_WITH(fmt_buffer, kFmtBufferSize, "%" PRIuLEAST32, column);
  } else {
    std::fputs("unknown", stderr);
  }

  std::fputs("]\n", stderr);

  std::fflush(stderr);

#if defined(STX_ENABLE_PANIC_BACKTRACE)

  std::fputs(
      "\nBacktrace:\nip: Instruction Pointer,  sp: Stack "
      "Pointer\n\n",
      stderr);

  int frames = backtrace::trace(
      [](backtrace::Frame frame, int i) {
        auto const print_none = []() { std::fputs("unknown", stderr); };

        auto const print_ptr = [](uintptr_t ip) {
          STX_PANIC_EPRINTF(internal::kxPtrFmtSize, "0x%" PRIxPTR, ip);
        };

        // int varies
        STX_PANIC_EPRINTF(internal::kI32FmtSize + 8, "#%d\t\t", i);

        frame.symbol.match(
            [](backtrace::Symbol& sym) {
              for (char c : sym.raw()) {
                std::fputc(c, stderr);
              }
            },
            print_none);

        std::fputs("\t (ip: ", stderr);

        frame.ip.match(print_ptr, print_none);

        std::fputs(", sp: ", stderr);

        frame.sp.match(print_ptr, print_none);

        std::fputs(")\n", stderr);

        return false;
      },
      1);

  if (frames <= 0) {
    std::fputs(
        R"(WARNING >> The stack frames couldn't be identified, debug information was possibly stripped, unavailable, or elided by compiler
)",
        stderr);
  }

  std::fputs("\n", stderr);

#endif

#if !defined(STX_NO_STD_THREAD_MUTEX)
  // other threads will still be able to log for some nanoseconds
  stderr_lock.unlock();
#endif
}

STX_END_NAMESPACE
