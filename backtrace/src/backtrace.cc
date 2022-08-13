/**
 * @file backtrace.cc
 * @author Basit Ayantunde <rlamarrr@gmail.com>
 * @date 2020-05-16
 *
 * @copyright MIT License
 *
 * Copyright (c) 2020-2022 Basit Ayantunde
 *
 */

#include "stx/backtrace.h"

#include <csignal>
#include <cstdint>
#include <cstdio>

#include "absl/debugging/stacktrace.h"
#include "absl/debugging/symbolize.h"

STX_BEGIN_NAMESPACE

// since backtracing will mostly be used in failure handling code, we can't make
// use of heap allocation
constexpr uint32_t MAX_STACK_FRAME_DEPTH = 128;

// MSVC and ICC typically have 1024 bytes max for a symbol
// The standard recommends 1024 bytes minimum for an identifier
constexpr uint32_t SYMBOL_BUFFER_SIZE = 1024;

std::string_view backtrace::Symbol::raw() const {
  return std::string_view{symbol_.data(), symbol_.size()};
}

int backtrace::trace(Fn<bool(Frame, int)> callback, int skip_count) {
  // TODO(lamarrr): get stack pointer in a portable and well-defined way

  void* ips[MAX_STACK_FRAME_DEPTH];
  uintptr_t sps[MAX_STACK_FRAME_DEPTH];
  int sizes[MAX_STACK_FRAME_DEPTH];

  (void)sps;

  int depth =
      absl::GetStackFrames(ips, sizes, MAX_STACK_FRAME_DEPTH, skip_count);

  if (depth <= 0) return 0;

  char symbol[SYMBOL_BUFFER_SIZE];
  int max_len = SYMBOL_BUFFER_SIZE;

  for (int i = 0; i < depth; i++) {
    symbol[0] = '\0';
    backtrace::Frame frame{};
    if (absl::Symbolize(ips[i], symbol, max_len)) {
      auto span = Span<char const>(symbol, max_len);
      frame.symbol = Some(backtrace::Symbol(std::move(span)));
    }

    frame.ip = Some(reinterpret_cast<uintptr_t>(ips[i]));

    if (callback(std::move(frame), depth - i)) break;
  }
  return depth;
}

STX_END_NAMESPACE
