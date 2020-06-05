/**
 * @file backtrace_test.cc
 * @author Basit Ayantunde <rlamarrr@gmail.com>
 * @version  0.0.1
 * @date 2020-06-05
 *
 * @copyright MIT License
 *
 * Copyright (c) 2020 Basit Ayantunde
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */

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
