
// disable backtrace on release builds?

#if defined(STX_UNWIND_UNSUPPORTED_PLATFORM)
#else

#include "cxxabi.h"    // check
#include "libunwind.h" // check in file and build
#endif

#include <stdio.h> // we need PRIxPTR :(

#ifdef NDEBUG // will this be om for embedded systems?

#include <cassert>

#define ASSERT_NEQ(x, y)                                                       \
  do {                                                                         \
    assert((x) != (y));                                                        \
  } while (false)

#define ASSERT_GT(x, y) >
do {
  > assert((x) > (y));
  >
} while (false)

#else
#define ASSERT_NEQ(x, y) 0
#define ASSERT_GT(x, y) 0
#endif

    //
    //
    //
    // Supports local unwinding only: i.e. within the current process
    //
    //
    //

    // make report

    // partial errors

    namespace stx {
  template <typename T, typename E> using Result = std::tuple<T, E>;
};

// the second path is for partial failures which shouldn't
//  be ignored either

// error occurred but here is the result
// using Error = Result<std::vector<FunctionInfo>

// format with PRIxPTR
struct Frame {
  Option<uintptr_t> instruction();   // instruction offset
  Option<uintptr_t> stack_pointer(); // address on the call stack
  Option<uintptr_t> address();       // function's address
  Option<Symbol> symbol(); // do we own this?, function symbol, can be null!
};

// bugs from us will be available for debug builds!

enum class DemangleError {
  PlatformUnsupported,
  InvalidSymbol,
  MallocFail, //?
  Disabled
};

struct FreeDeleter {

  void operator()(char *ptr) const noexcept { free(ptr); }
};

struct Symbol {

  auto demangle() -> Result<std::string, DemangleError> {

#if defined(STX_UNSUPPORTED_PLATFORM)
    return DemangleError::PlatformUnsupported;
#else

#if defined(STX_DISABLE_DEMANGLER)
    return i;

#else

    int status;
    char *str = abi::__cxxa_demangle(symbol_.get(), nullptr, 0, &status);

    ASSERT_NEQ(status, -3); // invalid parameters, should be valid
    ASSERT_NEQ(str, nullptr);

    switch (status) {
    case 0: {
      // success, memory was also allocated, so, non-null
      std::string name{str}; // no way to return if an exception is thrown
      free(str);
      return Ok(std::move(name));
    };
    case -1:
      return Err(DemanglerError::MallocFail);
    case -2:
      return Err(DemanglerError::InvalidSymbol);

#ifdef NDEBUG
    default:
      ASSERT_NEQ(status, status);
#endif
    };

#endif
#endif
#endif
  };

  // intended for common case quick use
  // i.e. frame.symbol().raw();, using a ref will be sublar
  std::string raw() const noexcept { return symbol_.get(); }

private:
  std::unique_ptr<char[], FreeDeleter> symbol_;

  // requires that `sym` is not a `nullptr` and is null-terminated
  explicit Symbol(char *sym) : symbol_{sym} {}
};

enum class SnapError { UnsupportedPlatform, Unknown, Disabled };

enum BacktraceError {
  UnsupportedPlatform

};

// represents current machine state at the callsite
// it should not be returned, copied, nor moved,hence the constructors have been
// made private. scopes as it would be invalidated in the process. it must be
// used immediately after creation.
// you can only pass by reference
struct Snapshot {

  static auto capture() -> Result<Snapshot, SnapError> {

#if defined(STX_UNWIND_UNSUPPORTED)

    return Err(CaptureError::UnsupportedPlatform);

#else

    Snapshot snap{};

    switch (unw_get_context(&snap.context_)) {
    case 0:
      return Ok(std::move(snap));
    default:
      return Err(Error::Unknown);
    };
#endif
  };

  // get a backtrace within the current snapshot
  // non-panicking
  // walks down the stack, and calls callback on each stack frame
  // as it does so. if callback evaluates to true, it stops.
  // else, it keeps walking the stack.
  auto trace(std::function<bool()(Frame)> callback) -> Option<BacktraceError> {

    // we might want to skip

#if defined(STX_UNWIND_UNSUPPORTED)

    return BacktraceError::UnsupportedPlatform;

#else
    unw_cursor_t cursor;
    unw_init_local(&cursor, &context_);

#endif
  }

private:
  Snapshot() : context_{} {}

  unw_context_t context_;
};

backtrace() {

  unw_cursor_t cursor{};
  unw_context_t context{};

  if (unw_getcontext(&context) != 0)
    return UnwindError::ContextInit;
  if ((auto err = unw_init_local(&cursor, &context)) == 0) {

    switch (err) {
    case UNW_EINVAL: // unlikely
      return UnwindError::Internal;
    case UNW_EUNSPEC:
      return UnwindError::Unspecified;
    case UNW_EBADREG:
      return UnwindError::Unspecified;
    default:
      return UnwindError::Internal;
    }
  }

  int n = 0;
  bool stop = false;
  std::vector<FunctionInfo> trace;

  while (!stop) {

    auto err = unw_step(&cursor);
    switch (err) {

    case UNW_EUNSPEC:
      return UnwindError::Unspecified;
    case UNW_ESTOPUNWIND:
      return std::move(trace);
    case UNW_EINVALID:
    case UNW_EBADFRAME:
    case UNW_EINVALIDIP:
case UNE
 return std::move(trace); // hmmmmm


};

    unw_word_t ip, sp, off;

    unw_get_reg(&cursor, UNW_REG_IP, &ip);
    unw_get_reg(&cursor, UNW_REG_SP, &sp);

    char symbol[256] = {"<unknown>"};
    char *name = symbol;

    if (!unw_get_proc_name(&cursor, symbol, sizeof(symbol), &off)) {
      int status;
      if ((name = abi::__cxa_demangle(symbol, NULL, NULL, &status)) == 0)
        name = symbol;
    }

    printf("#%-2d 0x%016" PRIxPTR " sp=0x%016" PRIxPTR " %s + 0x%" PRIxPTR "\n",
           ++n, static_cast<uintptr_t>(ip), static_cast<uintptr_t>(sp), name,
           static_cast<uintptr_t>(off));

    if (name != symbol) free(name);
  }

#endif
}

int c() {
  panic();
  return 0;
}
int b(int k = 89) {
  c();
  return k;
}
int a() {
  b();
  return 0;
}

int main() { a(); }
