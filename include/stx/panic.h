/**
 * @file panic.h
 * @author Basit Ayantunde <rlamarrr@gmail.com>
 * @brief
 * @version  0.1
 * @date 2020-04-26
 *
 * @copyright Copyright (c) 2020
 *
 */

#ifndef STX_PANIC_H_
#define STX_PANIC_H_

#include <string_view>

#ifdef STX_STABLE_SOURCE_LOCATION
#include <source_location>
#else
// it is already checked in the build file to see if source_location or
// experimental/source_location is present
#include <experimental/source_location>
#endif

// TODO(lamarrr): Add halting, aborting, and other widely used panic behaviors

namespace stx {

#ifdef STX_STABLE_SOURCE_LOCATION
using SourceLocation = std::source_location;
#else
using SourceLocation = std::experimental::source_location;
#endif

// here, we can avoid any form of memory allocation that might be needed,
// therefore deferring the info string to the callee and can also use a stack
// allocated string in cases where dynamic memory allocation is undesired
// the debugging breakpoint should be attached here
[[noreturn]] void panic_handler(std::string_view info, SourceLocation location);

/// This allows a program to terminate immediately and provide feedback to the
/// caller of the program. `panic` should be used when a program reaches an
/// unrecoverable state. This function is the perfect way to assert conditions
/// in example code and in tests. `panic` is closely tied with the `unwrap` and
/// `expect` method of both `Option` and `Result`. Both implementations call
/// `panic` when they are set to `None` or `Err` variants.
[[noreturn]] inline void panic(
    std::string_view info = "explicit panic",
    SourceLocation location = SourceLocation::current()) {
  panic_handler(std::move(info), std::move(location));
}
};  // namespace stx

#endif  // STX_PANIC_H_
