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
#include <string_view>

#include "stx/config.h"
#include "stx/fn.h"
#include "stx/option.h"
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

/// `Symbol` contains references to buffers and as such should not be copied nor
/// moved as a reference. Its raw data content can also be copied as a
/// `std::string`.
struct Symbol {
  /// gets the raw symbol name, the symbol is pre-demangled if possible.
  std::string_view raw() const;

  /// construct the `Symbol` object from the raw undemangled symbol name
  /// requires that `sym`'s `data` member is not a `nullptr` and is
  /// null-terminated.
  ///
  /// UNCHECKED!
  explicit Symbol(Span<char const> sym) : symbol_{sym} {};

 private:
  Span<char const> symbol_;
};

/// reperesents an active stack frame.
// all members are optional meaning the information they represent might not be
// available
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
///   stx::backtrace::trace(stx::fn::make_static([](Frame frame, int) {
///    std::cout << "Instruction Pointer="
///              << frame.ip.copy().unwrap()
///              << std::endl;
///    return false;
///  }));
///
/// ```
///
///
// all memory passed to the callback is cleared after each call. Hence we only
// use one stack memory for the callback feed-loop.
int trace(stx::Fn<bool(Frame, int)> const& callback, int skip_count = 0);

}  // namespace backtrace

STX_END_NAMESPACE
