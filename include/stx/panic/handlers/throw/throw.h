/**
 * @file throw.h
 * @author Basit Ayantunde <rlamarrr@gmail.com>
 * @brief
 * @version  0.1
 * @date 2020-04-26
 *
 * @copyright Copyright (c) 2020
 *
 */

#ifndef PANIC_HANDLERS_THROW_THROW_H_
#define PANIC_HANDLERS_THROW_THROW_H_
#include "stx/panic/handlers/throw/panic_info.h"

namespace stx {
/// Causes an exception of type `PanicInfo` to be thrown.
/// Exceptions thrown via panic_throw are not meant to be caught! They serve as
/// a termination point for the program
///
/// The `PanicInfo` exception contains a string description of the error, the
/// thread id of where the error occured and a source location referring to the
/// point where `panic_throw` was called.
[[noreturn]] inline void panic_throw(
    std::string_view info,
    SourceLocation location = SourceLocation::current()) {
  throw PanicInfo(std::move(info), std::move(location));
}
};  // namespace stx

#endif  // PANIC_HANDLERS_THROW_THROW_H_
