

#include "stx/backtrace.h"

#include "gtest/gtest.h"
#include "stx/option.h"
#include "stx/panic.h"

using namespace stx;
using namespace stx::backtrace;

// you forgetting msvc?

#define LOG(x) ::std::cout << (x) << ::std::endl

[[gnu::noinline]] void d() {
  std::cout << std::hex;

  backtrace::trace([](Frame frame, int) {
    frame.symbol.as_ref().match([](auto sym) { LOG(sym.get().raw()); },
                                []() { LOG("<unknown>\n"); });

    frame.ip.as_ref().match([](auto ip) { LOG("ip: " + std::to_string(ip)); },
                            []() {});

    return false;
  });

  stx::Option<int> f = stx::None;

  auto r = make_ok<int, int>(9);
  r.err_value();
  Ok h (9);
  int const t = std::forward<Ok<int> const&&>(std::move(h)).value();
}

[[gnu::noinline]] void c() { d(); }

[[gnu::noinline]] void b() { c(); }

[[gnu::noinline]] void a() { b(); }

TEST(BacktraceTest, Backtrace) { a(); }
