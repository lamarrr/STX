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

// config/backtrace.h
// config/common.h
// config/panic.h?
// config/monads.h

#define STX_SYMBOL_BUFFER_SIZE 2048UL

#if !defined(STX_DISABLE_BACKTRACE_DEMANGLER) && !defined(__cpp_exceptions)
#error \
    "Demangler backend requires exceptions and heap allocation, Please disable the demangler or turn on exceptions and heap allocation"
#endif

// debug-build assertions
#if defined(NDEBUG) && defined(STX_ENABLE_DEBUG_ASSERTIONS)
#include <cassert>
#define ASSERT_UNREACHABLE() assert(false)
#else
#define ASSERT_UNREACHABLE() (void)0
#endif

#include <array>
#include <cstdint>
#include <cstring>

#if !defined(STX_UNWIND_UNSUPPORTED_PLATFORM)
#define UNW_LOCAL_ONLY
#include "libunwind.h"
#endif

#if defined(STX_DISABLE_BACKTRACE_DEMANGLER) || \
    defined(STX_UNWIND_UNSUPPORTED_PLATFORM)
// we fill them with stubs
#else
#include "Demangle/Demangle.h"
#endif

#include <iostream>

#include "stx/backtrace.h"

#define LOG(x) \
  std::cout << "[" << __FILE__ << ":" << __LINE__ << "] " << x << std::endl

namespace stx {
namespace backtrace {

auto Symbol::demangle() -> Result<std::string, DemangleError> {
#if defined(STX_DISABLE_BACKTRACE_DEMANGLER)

  return Err(DemangleError::Disabled);

#else
#if defined(STX_UNWIND_UNSUPPORTED_PLATFORM)

  return Err(DemangleError::UnsupportedPlatform);

#else

  std::string raw_name{symbol_.data};

  std::string name = llvm::demangle(raw_name);

  // returns equal string if name is unknown
  if (name == raw_name) return Err(DemangleError::UnrecognizedSymbol);

  return Ok(std::move(name));

#endif
#endif
}

/// gets the raw undemangled symbol name
auto Symbol::raw() const noexcept -> std::string_view {
  return std::string_view(symbol_.data);
}

// get a backtrace within the current snapshot.
// walks down the stack, and calls callback on each stack frame
// as it does so. if callback evaluates to true, it stops.
// else, it keeps walking the stack.
// non-panicking.
// callback might throw an exception so, no noexcept.
// all memory passed to the callback is cleared after each call. Hence we only
// use one stack memory for the callback feed-loop.
auto trace(Callback callback) -> Option<BacktraceError> {
#if defined(STX_UNWIND_UNSUPPORTED_PLATFORM)

  return Some(BacktraceError::UnsupportedPlatform);

#else

  unw_context_t context;

  switch (unw_getcontext(&context)) {
    case 0:
      break;
    case -1:
      return Some(BacktraceError::Unknown);
    default:
      ASSERT_UNREACHABLE();  // smoke test
      return Some(BacktraceError::Unknown);
  }

  unw_cursor_t cursor{};

  switch (unw_init_local(&cursor, &context)) {
    case 0:

      break;
    case UNW_EINVAL:
      return Some(BacktraceError::RemoteMode);
    case UNW_EUNSPEC:
      return Some(BacktraceError::Unspecified);
    case UNW_EBADREG:
      return Some(BacktraceError::RegisterInaccessible);
    default:
      ASSERT_UNREACHABLE();  // smoke test
      return Some(BacktraceError::Unknown);
  }

  SymbolStorage symbol_storage;

  while (true) {
    int status = unw_step(&cursor);

    if (status >= 0) {
      if (status == 0) {
        // reached end of call stack

        break;
      }

      // positive is success
      // get frame info

      uintptr_t instr_p, stack_p, offset;
      Frame frame{};
      // let the errors slide
      switch (unw_get_reg(&cursor, UNW_REG_IP, &instr_p)) {
        case 0: {
          frame.instruction_pointer =
              make_some(instr_p);  // can't use intptr_t ?

          break;
        }
        case UNW_EBADREG:    // the register is inaccessible
        case UNW_EUNSPEC: {  // unspecified error
          // leave ip with None
          break;
        }
        default:
          ASSERT_UNREACHABLE();  // smoke test
          return Some(BacktraceError::Unknown);
      }

      switch (unw_get_reg(&cursor, UNW_REG_SP, &stack_p)) {
        case 0: {
          frame.stack_pointer = make_some(stack_p);

          break;
        }
        case UNW_EBADREG:    // the register is inaccessible
        case UNW_EUNSPEC: {  // unspecified error
                             // leave sp with None

          break;
        }

        default:
          ASSERT_UNREACHABLE();  // smoke test
          return Some(BacktraceError::Unknown);
      }

      switch (unw_get_proc_name(&cursor, symbol_storage.data(),
                                symbol_storage.size(), &offset)) {
        case 0: {
          frame.symbol = Some(Symbol(CharSpan(symbol_storage)));
          break;
        }
        case UNW_ENOINFO: {
          // leave sym with None

          break;
        }
        case UNW_EUNSPEC: {
          // leave sym with None

          break;
        }

        default:
          ASSERT_UNREACHABLE();  // smoke test
          return Some(BacktraceError::Unknown);
      }

      frame.offset = Some(std::move(offset));

      if (callback(Ok(std::move(frame)))) return None;

      std::memset(symbol_storage.data(), 0, symbol_storage.size());

    } else {
      // negative (failure)
      switch (status) {
        case UNW_EUNSPEC:
          callback(Err(WalkError::Unspecified));  // try to continue
          break;

        case UNW_ENOINFO:
          callback(Err(WalkError::UnrecognizedUnwindInfo));  // try to continue
          break;

        case UNW_EINVALIDIP:
          callback(
              Err(WalkError::InvalidInstructionPointer));  // try to continue
          break;

        case UNW_EBADFRAME:
          callback(Err(WalkError::NextFrameInvalid));  // mustn't proceed
          return None;

        case UNW_ESTOPUNWIND:
          callback(Err(WalkError::BackendStopped));  // stop
          return None;

        default:
          ASSERT_UNREACHABLE();  // smoke test
          return Some(BacktraceError::Unknown);
      }
    }
  }

  // should never reach here
  ASSERT_UNREACHABLE();  // smoke test

  return None;
#endif
}
};  // namespace backtrace

};  // namespace stx
