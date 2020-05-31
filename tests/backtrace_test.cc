

#include "stx/backtrace.h"

#include "gtest/gtest.h"
#include "stx/option.h"
#include "stx/panic.h"

using namespace stx;
using namespace stx::backtrace;

// you forgetting msvc?

#define LOG(x) ::std::cout << (x) << ::std::endl

[[gnu::noinline]] void fn_d() {
  std::cout << std::hex;

  backtrace::trace([](Frame frame, int) {
    frame.symbol.as_ref().match([](auto sym) { LOG(sym.get().raw()); },
                                []() { LOG("<unknown>\n"); });

    frame.ip.as_ref().match([](auto ip) { LOG("ip: " + std::to_string(ip)); },
                            []() {});

    return false;
  });
}

[[gnu::noinline]] void fn_c() { fn_d(); }

[[gnu::noinline]] void fn_b() { fn_c(); }

[[gnu::noinline]] void fn_a() { fn_b(); }

TEST(BacktraceTest, Backtrace) { fn_a(); }
