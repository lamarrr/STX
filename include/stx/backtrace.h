#pragma once
/**
 * @file backtrace.h
 * @author Basit Ayantunde <rlamarrr@gmail.com>
 * @brief
 * @version  0.1
 * @date 2020-05-13
 *
 * @copyright Copyright (c) 2020
 *
 */
#include <array>
#include <functional>
#include <string>

#include "stx/option.h"
#include "stx/result.h"

#define STX_SYMBOL_BUFFER_SIZE 2048UL

namespace stx {
// generally not intended for embedded systems.
// thread-safe.
// locking and others.

//!
//!
//!
//! Supports local unwinding only: i.e. within the current process
//!
//!
//!

// NOTE: All these functions are fallible especially because not all platforms
// support them, hence for unsupported platforms `Error::PlatformUnsupported` is
// returned for function calls that require them.

namespace backtrace {

enum class DemangleError {
  UnsupportedPlatform,
  UnrecognizedSymbol,  // can only demangle Itanium and MSVC ABIs
  Disabled,
  InvalidSymbol  // UNW_EINVAL
};

enum class BacktraceError {
  UnsupportedPlatform,
  RemoteMode,            // UNW_EINVAL
  RegisterInaccessible,  // UNW_BADREG
  Unspecified,           // UNW_EUNSPEC - unspecified (general) error
  Unknown,               // A bug
};

enum class WalkError {
  Unspecified,
  NoInfo,
  UnrecognizedUnwindInfo,
  InvalidInstructionPointer,
  NextFrameInvalid,
  BackendStopped,
  Unknown,
};

// not available yet :(
struct CharSpan {
  char *data;
  size_t size;
  template <size_t N>
  explicit CharSpan(std::array<char, N> &container) noexcept  // NOLINT
      : data{container.data()}, size{container.size()} {}
  CharSpan(CharSpan const &) noexcept = default;
  CharSpan(CharSpan &&) noexcept = default;
  CharSpan &operator=(CharSpan const &) noexcept = default;
  CharSpan &operator=(CharSpan &&) noexcept = default;
  ~CharSpan() noexcept = default;
};

using SymbolStorage =
    std::array<char, STX_SYMBOL_BUFFER_SIZE>;  // the MSVC and ICC typically
                                               // have 2048 max symbol length
struct Symbol {
  auto demangle() -> Result<std::string, DemangleError>;
  auto raw() const noexcept -> std::string_view;

  /// construct the `Symbol` object from the raw undemangled symbol name
  /// requires that `sym`'s `data` member is not a `nullptr` and is
  /// null-terminated.
  ///
  /// UNCHECKED!
  explicit Symbol(CharSpan sym) noexcept : symbol_{sym} {};

  char *c_str() { return symbol_.data; }

 private:
  CharSpan symbol_;
};

struct Frame {
  Option<uintptr_t> instruction_pointer;  // instruction pointer
  Option<uintptr_t> stack_pointer;        // address on the call stack
  Option<uintptr_t> offset;  // offset of the function's call-site to the
                             // address of the callee, same as on the call stack
  Option<Symbol> symbol;

  explicit Frame() noexcept  // NOLINT
      : instruction_pointer{None},
        stack_pointer{None},
        offset{None},
        symbol{None} {}
  Frame(Frame &&) = default;
  Frame &operator=(Frame &&) = default;

  Frame(Frame const &cp)
      : instruction_pointer{cp.instruction_pointer.clone()},
        stack_pointer{cp.stack_pointer.clone()},
        offset{cp.offset.clone()},
        symbol{cp.symbol.clone()} {}

  Frame &operator=(Frame const &cp) {
    instruction_pointer = cp.instruction_pointer.clone();
    stack_pointer = cp.stack_pointer.clone();
    offset = cp.offset.clone();
    symbol = cp.symbol.clone();
    return *this;
  }
};

using Callback = std::function<bool(Result<Frame, WalkError>)>;

auto trace(Callback callback) -> Option<BacktraceError>;

/// `Symbol` contains references to buffers and as such should not be copied nor
/// moved as a reference. Its raw data content can also be copied as a
/// `std::string`.

// handles the specified signals.
// also provides a backtrace.
// the user can attach this at program startup.
// void install_signal_handler(int signal) noexcept;

// handles the all known signals: SIGABRT, SIGSEGV, SIG... (etc) and also
// provides a backtrace the user can attach this at program startup
// void install_signal_handler() noexcept;

};  // namespace backtrace
};  // namespace stx
