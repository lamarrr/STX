/**
 * @file backtrace.cc
 * @author Basit Ayantunde <rlamarrr@gmail.com>
 * @version  0.0.1
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

#include "stx/backtrace.h"

#include <csignal>
#include <cstdio>

#include "absl/debugging/stacktrace.h"
#include "absl/debugging/symbolize.h"

namespace stx {

auto backtrace::Symbol::raw() const noexcept -> std::string_view {
  return std::string_view(symbol_.data);
}

int backtrace::trace(Callback callback, int skip_count) {

  // auto head_sp = static_cast<uintptr_t>(&stack_head);

  void* ips[STX_MAX_STACK_FRAME_DEPTH];
  //  uintptr_t sps[STX_MAX_STACK_FRAME_DEPTH];
  int sizes[STX_MAX_STACK_FRAME_DEPTH];

  int depth =
      absl::GetStackFrames(ips, sizes, STX_MAX_STACK_FRAME_DEPTH, skip_count);

  if (depth <= 0) return 0;

  char symbol[STX_SYMBOL_BUFFER_SIZE];
  int max_len = STX_SYMBOL_BUFFER_SIZE;

  // uintptr_t stack_ptr = head_sp;

  for (int i = 0; i < depth; i++) {
    symbol[0] = '\0';
    Frame frame{};
    if (absl::Symbolize(ips[i], symbol, max_len)) {
      auto span = backtrace::CharSpan(symbol, max_len);
      frame.symbol = Some(backtrace::Symbol(std::move(span)));
    }

    // stack_ptr += static_cast<uintptr_t>(sizes[i]);

    frame.ip = Some(reinterpret_cast<uintptr_t>(ips[i]));

    // frame.sp = Some(static_cast<uintptr_t>(stack_ptr));

    if (callback(std::move(frame), depth - i)) break;
  }

  return depth;
}

namespace {

void print_backtrace() {
  fputs(
      "\n\nBacktrace:\nip: Instruction Pointer,  sp: Stack "
      "Pointer\n\n",
      stderr);

  int frames = backtrace::trace(
      [](backtrace::Frame frame, int i) {
      fprintf(stderr, "0x%" PRIxPTR, ptr);
    };

    fprintf(stderr, "#%d\t\t", i);

    frame.symbol.match(
        [](auto& sym) {
          for (char c : sym.raw()) {
            fputc(c, stderr);
          }
        },
        print_none);

    fputs("\t (ip: ", stderr);

    frame.ip.match(print_ptr, print_none);

    fputs(", sp: ", stderr);

    frame.sp.match(print_ptr, print_none);

    fputs(")\n", stderr);

    return false;

  if (frames == 0) {
    std::fputs(
        R"(WARNING >> The stack frames couldn't be identified, debug information was possibly stripped, unavailable, or elided by compiler
)",
        stderr);
  }

  fputs("\n", stderr);
}

[[noreturn]] void signal_handler(int signal) {
  fputs("\n\n", stderr);
  switch (signal) {
    case SIGSEGV:
      fputs(
          "Received 'SIGSEGV' signal. Invalid memory access occurred "
          "(segmentation fault).",
          stderr);
      break;
    case SIGILL:
      fputs(
          "Received 'SIGILL' signal. Invalid program image (illegal/invalid "
          "instruction, i.e. nullptr dereferencing).",
          stderr);
      break;
    case SIGFPE:
      fputs(
          "Received 'SIGFPE' signal. Erroneous arithmetic operation (i.e. "
          "divide by zero).",
          stderr);
      break;
  }

  print_backtrace();
  std::abort();
}
}  // namespace

auto backtrace::handle_signal(int signal) noexcept
    -> Result<void (*)(int), SignalError> {
  if (signal != SIGSEGV && signal != SIGILL && signal != SIGFPE)
    return Err(SignalError::Unknown);

  auto err = std::signal(signal, signal_handler);

  if (err == SIG_ERR) return Err(SignalError::SigErr);

  return Ok(std::move(err));
}

}  // namespace stx
