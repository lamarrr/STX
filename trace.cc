// config file
#define STX_ENABLE_BACKTRACE

#if defined(STX_ENABLE_BACKTRACE)  // also check for linker flags
#if __has_include("libunwind.h")
#define STX_HAS_LIB_UNWIND
#else
#error Can not find libunwind in include path. install "libunwind-dev" via your package manager, OR  build and install from source @ https://github.com/libunwind/libunwind \
To disable backtrace set `STX_ENABLE_BACKTRACE` to `OFF` in your CMakeLists.txt file
#endif
#endif

// different on MSVC
// is it available on msvc?
#if !defined(_MSC_VER)
#if __has_include("cxxabi.h")
#define STX_HAS_CXXABI_HEADER
#else
#error Can not find the C++ ABI header (cxxabi.h)
#endif
#else
// #if __has_include???
#endif

#if defined(STX_ENABLE_BACKTRACE) && defined(STX_HAS_LIB_UNWIND)
#include "libunwind.h"  // check in file and build
#endif

#if defined(STX_ENABLE_BACKTRACE) && defined(STX_HAS_CXXABI_HEADER)
#include "cxxabi.h"
#endif

#include <cstdio>
#include <cstdlib>
#include <string>

// provide ABI for user to override?
std::string get_backtrace() {
#if defined(STX_ENABLE_BACKTRACE)
  // note: run with `STX_NO_BACKTRACE=1` environment variable to prevent
  // displaying a backtrace
  char const* env_value = std::getenv("STX_NO_BACKTRACE");

  bool print_full = false;

  if (env_value == nullptr || *env_value != '1') {
    fputs("Printing backtrace...\n", stderr);
  }

  unw_cursor_t cursor{};
  unw_context_t context{};
  // handle
  unw_getcontext(&context);
  // handle
  unw_init_local(&cursor, &context);

  // \t Instruction Pointer,  \t Stack Pointer \t function name

  int n = 0;
  while (unw_step(&cursor) != 0) {
    unw_word_t ip{}, sp{}, off{};

    // handle
    // instruction pointer
    unw_get_reg(&cursor, UNW_REG_IP, &ip);
    // handle
    // stack pointer
    unw_get_reg(&cursor, UNW_REG_SP, &sp);
    // size?
    char symbol[512] = "<unknown>";
    char* name = symbol;

    if (!unw_get_proc_name(&cursor, symbol, sizeof(symbol), &off)) {
      int status;

#if defined(STX_HAS_CXXABI_HEADER)

      if ((name = abi::__cxa_demangle(symbol, NULL, NULL, &status)) == 0)
        name = symbol;
    }

#endif
    // handle
    printf("#%-2d 0x%016" PRIxPTR " sp=0x%016" PRIxPTR " %s + 0x%" PRIxPTR "\n",
           ++n, static_cast<uintptr_t>(ip), static_cast<uintptr_t>(sp), name,
           static_cast<uintptr_t>(off));

    if (name != symbol) free(name);
  }
  return "";

#endif
}

int c() {
  get_backtrace();
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
