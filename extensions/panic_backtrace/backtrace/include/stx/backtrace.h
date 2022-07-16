/**
 * @file backtrace.h
 * @author Basit Ayantunde <rlamarrr@gmail.com>
 * @date 2020-05-13
 *
 * @copyright MIT License
 *
 * Copyright (c) 2020-2022 Basit Ayantunde
 *
 */

#pragma once

#include <cstdint>

#include "stx/internal/option_result.h"
#include "stx/report.h"
#include "stx/span.h"

//! @file
//!
//! - Thread and signal-safe, non-allocating.
//! - Supports local backtracing only: i.e. within the current process
//!
//!
//!

STX_BEGIN_NAMESPACE

namespace backtrace {

enum class SignalError {
  /// An Unknown error occurred
  Unknown,
  /// `std::signal` returned `SIG_ERR`
  SigErr
};

inline SpanReport operator>>(ReportQuery, SignalError const &err) {
  switch (err) {
    case SignalError::Unknown:
      return SpanReport(
          "Uknown signal given, 'handle_signal' can only handle 'SIGSEGV', "
          "'SIGILL' and 'SIGFPE'.");
    case SignalError::SigErr:
      return SpanReport("'std::signal' returned 'SIGERR'");
    default:
      return SpanReport();
  }
}

/// `Symbol` contains references to buffers and as such should not be copied nor
/// moved as a reference. Its raw data content can also be copied as a
/// `std::string`.
struct Symbol {
  /// gets the raw symbol name, the symbol is pre-demangled if possible.
  auto raw() const -> std::string_view;

  /// construct the `Symbol` object from the raw undemangled symbol name
  /// requires that `sym`'s `data` member is not a `nullptr` and is
  /// null-terminated.
  ///
  /// UNCHECKED!
  explicit Symbol(Span<char> sym) : symbol_{sym} {};

 private:
  Span<char> symbol_;
};

/// reperesents an active stack frame.
struct Frame {
  /// instruction pointer
  Option<uintptr_t> ip;
  /// address on the call stack
  Option<uintptr_t> sp;
  /// offset of the function's call-site to the callee on the instruction block.
  Option<uintptr_t> offset;
  /// function's symbol name. possibly demangled.
  Option<Symbol> symbol;
};

using Callback = bool (*)(Frame, int);

/// Gets a backtrace within the current machine's state.
/// This function walks down the stack, and calls callback on each stack frame
/// as it does so. if the callback invocation's result evaluates to true, it
/// stops which means it has found the desired stack frame, else, it keeps
/// walking the stack until it exhausts the stack frame / reaches the maximum
/// stack depth. This function does not perform stack-unwinding.
///
/// Returns the number of stack frames read.
///
///
/// `callback`  is allowed to throw exceptions.
///
/// # WARNING
///
/// Do not move the frame nor its member objects out of the
/// callback, nor bind a reference to them.
///
///
/// # Panic and Exception Tolerance
///
/// The function is non-panicking as long as the callback doesn't panic. The
/// callback can throw an exception.
///
///
///
/// # Example
///
/// ```cpp
///
///   stx::backtrace::trace([](Frame frame, int) {
///    std::cout << "Instruction Pointer="
///              << frame.ip.copy().unwrap()
///              << std::endl;
///    return false;
///  });
///
/// ```
///
///
// all memory passed to the callback is cleared after each call. Hence we only
// use one stack memory for the callback feed-loop.
int trace(Callback callback, int skip_count = 0);

/// Installs an handler for the specified signal that prints a backtrace
/// whenever the signal is raised. It can and will only handle `SIGSEGV`,
/// `SIGILL`, and `SIGFPE`. It returns the previous signal handler if
/// successful, else returns the error.
///
/// # Example
///
/// ``` cpp
/// // immediately after program startup
/// handle_signal(SIGILL).unwrap();
/// handle_signal(SIGSEGV).unwrap();
/// handle_signal(SIGFPE).unwrap();
///
/// ```
///
auto handle_signal(int signal) -> Result<void (*)(int), SignalError>;

}  // namespace backtrace

STX_END_NAMESPACE
