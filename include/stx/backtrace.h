/**
 * @file backtrace.h
 * @author Basit Ayantunde <rlamarrr@gmail.com>
 * @date 2020-05-13
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

#include <cstdint>

#include "stx/internal/option_result.h"
#include "stx/report.h"

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

inline SpanReport operator>>(ReportQuery, SignalError const &err) noexcept {
  switch (err) {
    case SignalError::Unknown:
      return SpanReport(
          "Uknown signal given, 'handle_signal' can only handle 'SIGSEGV', "
          "'SIGILL' and 'SIGFPE'.");
    case SignalError::SigErr:
      return SpanReport("'std::signal' returned 'SIGERR'");
  }
}

/// Mutable type-erased view over a contiguous character container.
struct CharSpan {
  char *data;
  size_t size;
  CharSpan(char *data_, size_t size_) noexcept : data{data_}, size{size_} {}
  CharSpan(CharSpan const &) noexcept = default;
  CharSpan(CharSpan &&) noexcept = default;
  CharSpan &operator=(CharSpan const &) noexcept = default;
  CharSpan &operator=(CharSpan &&) noexcept = default;
  ~CharSpan() noexcept = default;
};

/// `Symbol` contains references to buffers and as such should not be copied nor
/// moved as a reference. Its raw data content can also be copied as a
/// `std::string`.
struct Symbol {
  /// gets the raw symbol name, the symbol is pre-demangled if possible.
  auto raw() const noexcept -> std::string_view;

  /// construct the `Symbol` object from the raw undemangled symbol name
  /// requires that `sym`'s `data` member is not a `nullptr` and is
  /// null-terminated.
  ///
  /// UNCHECKED!
  explicit Symbol(CharSpan sym) noexcept : symbol_{sym} {};

 private:
  CharSpan symbol_;
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

  constexpr explicit Frame() = default;
  Frame(Frame &&) = default;
  Frame &operator=(Frame &&) = default;
  Frame(Frame const &) = default;
  Frame &operator=(Frame const &) = default;
  ~Frame() = default;
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
///              << frame.ip.clone().unwrap()
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
auto handle_signal(int signal) noexcept -> Result<void (*)(int), SignalError>;

}  // namespace backtrace

STX_END_NAMESPACE
