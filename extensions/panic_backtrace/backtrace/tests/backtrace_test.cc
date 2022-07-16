/**
 * @file backtrace_test.cc
 * @author Basit Ayantunde <rlamarrr@gmail.com>
 * @date 2020-06-05
 *
 * @copyright MIT License
 *
 * Copyright (c) 2020-2022 Basit Ayantunde
 *
 */

#include "stx/backtrace.h"

#include "gtest/gtest.h"
#include "stx/option.h"
#include "stx/panic.h"

using namespace stx;             // NOLINT
using namespace stx::backtrace;  // NOLINT

void fn_d() {
  backtrace::trace(
      [](Frame frame, int) {
        frame.symbol.match(
            [](auto sym) {
              auto const& s = sym.raw();
              std::fwrite(s.data(), s.size(), 1, stdout);
            },
            []() { std::fputs("unknown symbol", stdout); });

        std::puts("");

        frame.ip.match([](auto ip) { std::printf("ip: 0x%" PRIxPTR, ip); },
                       []() { std::fputs("unknown pc", stdout); });

        puts("\n");
        return false;
      },
      1);
}

void fn_c() { fn_d(); }

void fn_b() { fn_c(); }

void fn_a() { fn_b(); }

TEST(BacktraceTest, Backtrace) { fn_a(); }
