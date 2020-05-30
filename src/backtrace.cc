/**
 * @file backtrace.cc
 * @author Basit Ayantunde <rlamarrr@gmail.com>
 * @brief
 * @version  0.1
 * @date 2020-05-16
 *
 * @copyright Copyright (c) 2020
 *
 */

// debug-build assertions
#if !defined(NDEBUG) && defined(STX_ENABLE_DEBUG_ASSERTIONS)
#include <cassert>
#include <string>

using namespace std::literals;

#define LOG(x) \
  std::cout << "[" << __FILE__ << ":" << __LINE__ << "] " << x << std::endl
#define ASSERT_UNREACHABLE()        \
  LOG("Source location reached"sv); \
  assert(false)

#define ASSERT_NEQ(a, b)                                                    \
  if (!((a) != (b))) {                                                      \
    LOG("Assertion: '"s + std::to_string(a) + " != "s + std::to_string(b) + \
        "' failed"s);                                                       \
    assert((a) != (b));                                                     \
  }
#define ASSERT_EQ(a, b)                                                     \
  if (!((a) == (b))) {                                                      \
    LOG("Assertion: '"s + std::to_string(a) + " == "s + std::to_string(b) + \
        "' failed"s);                                                       \
    assert((a) == (b));                                                     \
  }
#else
#define LOG(x) (void)0
#define ASSERT_UNREACHABLE() (void)0
#define ASSERT_NEQ(a, b) (void)0
#define ASSERT_EQ(a, b) (void)0
#endif

#include <stdio.h>

#include <array>
#include <csignal>
#include <cstring>
#include <iostream>

#include "absl/debugging/stacktrace.h"
#include "absl/debugging/symbolize.h"
#include "stx/backtrace.h"

namespace stx {

auto backtrace::Symbol::raw() const noexcept -> std::string_view {
  return std::string_view(symbol_.data);
}

size_t backtrace::trace(Callback callback) {
  // size_t stack_head = 0;
  // *static_cast<volatile size_t*>(&stack_head) = 2;

  // auto head_sp = static_cast<uintptr_t>(&stack_head);

  int skip_count = 0;
  void* ips[STX_MAX_STACK_FRAME_DEPTH] = {};
  uintptr_t sps[STX_MAX_STACK_FRAME_DEPTH] = {};
  int sizes[STX_MAX_STACK_FRAME_DEPTH] = {};

  int depth = absl::GetStackFrames(ips, sizes, sizeof(ips) / sizeof(ips[0]),
                                   skip_count);

  char symbol[STX_SYMBOL_BUFFER_SIZE] = {};
  auto max_len = sizeof(symbol) / sizeof(symbol[0]);

  // uintptr_t stack_ptr = head_sp;
  (void)sps;

  for (int i = 0; i < depth; i++) {
    std::memset(symbol, 0, max_len);
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

  backtrace::trace([](backtrace::Frame frame, int i) {
    auto const print_none = []() { fputs("<unknown>", stderr); };
    auto const print_ptr = [](Ref<uintptr_t> ptr) {
      fprintf(stderr, "0x%" PRIxPTR, ptr.get());
    };

    fprintf(stderr, "#%d\t\t", i);

    frame.symbol.as_ref().match(
        [](Ref<backtrace::Symbol> sym) {
          for (char c : sym.get().raw()) {
            fputc(c, stderr);
          }
        },
        print_none);

    fputs("\t (ip: ", stderr);

    frame.ip.as_ref().match(print_ptr, print_none);

    fputs(", sp: ", stderr);

    frame.sp.as_ref().match(print_ptr, print_none);

    fputs(")\n", stderr);

    return false;
  });

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

};  // namespace stx
