/**
 * @file print.h
 * @author Basit Ayantunde <rlamarrr@gmail.com>
 * @date 2020-06-07
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

#pragma once

#include <cstdio>

/// @file
///
/// Safe snprintfs

/// printf using a provided buffer
#define STX_PANIC_EPRINTF_WITH(STX_ARG_BUFFER, STX_ARG_BUFFER_SIZE,     \
                               STX_ARG_FORMAT, STX_ARG_VALUE)           \
  {                                                                     \
    int fmt_size = ::std::snprintf(STX_ARG_BUFFER, STX_ARG_BUFFER_SIZE, \
                                   STX_ARG_FORMAT, STX_ARG_VALUE);      \
    if (fmt_size >= STX_ARG_BUFFER_SIZE) {                              \
      ::std::fputs("<format buffer insufficient>", stderr);             \
    } else if (fmt_size < 0) {                                          \
      ::std::fputs("<format implementation error>", stderr);            \
    } else {                                                            \
      ::std::fputs(STX_ARG_BUFFER, stderr);                             \
    };                                                                  \
  }

/// printf, makes own buffer
#define STX_PANIC_EPRINTF(STX_ARG_STR_SIZE, STX_ARG_FORMAT, STX_ARG_VALUE) \
  {                                                                        \
    /* string length + terminating null character */                       \
    char fmt_buffer[STX_ARG_STR_SIZE + 1];                                 \
    int fmt_size = ::std::snprintf(fmt_buffer, STX_ARG_STR_SIZE + 1,       \
                                   STX_ARG_FORMAT, STX_ARG_VALUE);         \
    if (fmt_size >= STX_ARG_STR_SIZE) {                                    \
      ::std::fputs("<format buffer insufficient>", stderr);                \
    } else if (fmt_size < 0) {                                             \
      ::std::fputs("<format implementation error>", stderr);               \
    } else {                                                               \
      ::std::fputs(fmt_buffer, stderr);                                    \
    };                                                                     \
  }
