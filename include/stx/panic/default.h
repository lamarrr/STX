/**
 * @file default.h
 * @author Basit Ayantunde <rlamarrr@gmail.com>
 * @date 2020-05-22
 *
 * @copyright MIT License
 *
 * Copyright (c) 2020-2022 Basit Ayantunde
 *
 */

#pragma once

#if !defined(STX_NO_STD_THREAD)
#  include <thread>
#endif

#include "stx/backtrace.h"
#include "stx/fn.h"
#include "stx/panic.h"
#include "stx/panic/print.h"
#include "stx/spinlock.h"

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
// at once as the buffer can likely not be enough.
//
inline void panic_default(std::string_view info, std::string_view error_report, SourceLocation location)
{
  // probably too much, but enough
  // this will at least hold a formatted uint128_t (40 digits)
  static constexpr int const FMT_BUFFER_SIZE = 64;

#if !defined(STX_NO_STD_THREAD)

  constexpr auto const THREAD_ID_HASHER = std::hash<std::thread::id>{};

  size_t const thread_id_hash = THREAD_ID_HASHER(std::this_thread::get_id());

#endif

  static SpinLock stderr_lock;

  LockGuard guard{stderr_lock};

  std::fputs("\nthread", stderr);

#if !defined(STX_NO_STD_THREAD)

  std::fputs(" with hash: '", stderr);

  STX_PANIC_EPRINTF(FMT_BUFFER_SIZE, "%zu", thread_id_hash);

  std::fputs("' ", stderr);

#endif

  std::fputs(" panicked with: '", stderr);

  for (char c : info)
  {
    std::fputc(c, stderr);
  }

  if (!error_report.empty())
  {
    std::fputs(": ", stderr);

    for (auto c : error_report)
    {
      std::fputc(c, stderr);
    }
  }

  std::fputs("' at function: '", stderr);

  std::fputs(location.function, stderr);

  std::fputs("' [", stderr);

  std::fputs(location.file, stderr);

  std::fputc(':', stderr);

  auto line = location.line;

  if (line != 0)
  {
    STX_PANIC_EPRINTF(FMT_BUFFER_SIZE, "%" PRIuLEAST32, line);
  }
  else
  {
    std::fputs("unknown", stderr);
  }

  std::fputc(':', stderr);

  auto column = location.column;

  if (column != 0)
  {
    STX_PANIC_EPRINTF(FMT_BUFFER_SIZE, "%" PRIuLEAST32, column);
  }
  else
  {
    std::fputs("unknown", stderr);
  }

  std::fputs("]\n", stderr);

  std::fflush(stderr);

  std::fputs(
      "\nBacktrace:\nip: Instruction Pointer,  sp: Stack "
      "Pointer\n\n",
      stderr);

  int frames = backtrace::trace(
      stx::fn::make_static([](backtrace::Frame frame, int i) {
        auto const print_none = []() { std::fputs("unknown", stderr); };

        auto const print_ptr = [](uintptr_t ip) {
          STX_PANIC_EPRINTF(FMT_BUFFER_SIZE, "0x%" PRIxPTR, ip);
        };

        STX_PANIC_EPRINTF(FMT_BUFFER_SIZE, "#%d\t\t", i);

        frame.symbol.match(
            [](backtrace::Symbol const &sym) {
              for (char c : sym.raw())
              {
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
      }),
      1);

  if (frames <= 0)
  {
    std::fputs(
        R"(WARNING >> The stack frames couldn't be identified, debug information was possibly stripped, unavailable, or elided by compiler)",
        stderr);
  }

  std::fputs("\n", stderr);
  std::fflush(stderr);
}

STX_END_NAMESPACE
