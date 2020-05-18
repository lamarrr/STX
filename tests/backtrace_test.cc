

#include "stx/backtrace.h"

#include "gtest/gtest.h"
#include "stx/option.h"
#include "stx/panic.h"

using namespace stx;

[[gnu::noinline]] void d() {
  backtrace::trace([](auto frame) {
    std::move(frame).match(
        [](auto r) {
          if (r.symbol.is_none()) {
            uintptr_t x = 00;
            std::cout << "<inlined>\t" << std::hex
                      << r.instruction_pointer.as_ref().unwrap().get() << "\n";
          } else {
            std::cout << r.symbol.as_ref().unwrap().get().raw() << std::endl;
          }
        },
        [](auto) { std::cout << "Error" << std::endl; });

    return false;
  }).unwrap_none();
}

[[gnu::noinline]] void c() {
  volatile int x = 0;
  while (x != 0)
    ;
  d();
}

[[gnu::noinline]] void b() { c(); }

[[gnu::noinline]] void a() { b(); }

TEST(BacktraceTest, Backtrace) { a(); }
