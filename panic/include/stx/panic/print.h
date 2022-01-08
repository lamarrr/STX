/**
 * @file print.h
 * @author Basit Ayantunde <rlamarrr@gmail.com>
 * @date 2020-06-07
 *
 * @copyright MIT License
 *
 * Copyright (c) 2020-2021 Basit Ayantunde
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
