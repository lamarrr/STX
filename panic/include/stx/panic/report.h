/**
 * @file report.h
 * @author Basit Ayantunde <rlamarrr@gmail.com>
 * @date 2020-05-22
 *
 * @copyright MIT License
 *
 * Copyright (c) 2020-2022 Basit Ayantunde
 *
 */

#pragma once

#include <cinttypes>
#include <cstddef>
#include <cstdio>
#include <string>
#include <string_view>

#include "stx/utils/common.h"

STX_BEGIN_NAMESPACE

constexpr std::string_view REPORT_FORMAT_ERROR = "<format error>";

struct ReportQuery {
  struct Buffer {
    char* data = nullptr;
    size_t size = 0;
  };

  Buffer buffer;
};

constexpr ReportQuery report_query{};

template <typename T>
[[nodiscard]] std::string_view operator>>(ReportQuery, T const& value) {
  (void)value;

  return {};
}

#define STX_INTERNAL_MAKE_REPORT_(STX_ARG_BUFFER, STX_ARG_FORMAT,        \
                                  STX_ARG_VALUE)                         \
  if (STX_ARG_BUFFER.size == 0 || STX_ARG_BUFFER.data == nullptr)        \
    return std::string_view{};                                           \
                                                                         \
  int fmt_size = std::snprintf(STX_ARG_BUFFER.data, STX_ARG_BUFFER.size, \
                               STX_ARG_FORMAT, STX_ARG_VALUE);           \
                                                                         \
  if (fmt_size < 0) {                                                    \
    return REPORT_FORMAT_ERROR;                                          \
  } else {                                                               \
    return std::string_view{STX_ARG_BUFFER.data,                         \
                            static_cast<size_t>(fmt_size)};              \
  }

template <typename T>
[[nodiscard]] inline std::string_view operator>>(ReportQuery query,
                                                 T const* const& ptr) {
  STX_INTERNAL_MAKE_REPORT_(query.buffer, "0x%" PRIxPTR,
                            reinterpret_cast<uintptr_t>(ptr));
}

template <typename T>
[[nodiscard]] inline std::string_view operator>>(ReportQuery query,
                                                 T* const& ptr) {
  STX_INTERNAL_MAKE_REPORT_(query.buffer, "0x%" PRIxPTR,
                            reinterpret_cast<uintptr_t>(ptr));
}

[[nodiscard]] inline std::string_view operator>>(ReportQuery query,
                                                 int8_t const& value) {
  STX_INTERNAL_MAKE_REPORT_(query.buffer, "%" PRIi8, value);
}

[[nodiscard]] inline std::string_view operator>>(ReportQuery query,
                                                 uint8_t const& value) {
  STX_INTERNAL_MAKE_REPORT_(query.buffer, "%" PRIu8, value);
}

[[nodiscard]] inline std::string_view operator>>(ReportQuery query,
                                                 int16_t const& value) {
  STX_INTERNAL_MAKE_REPORT_(query.buffer, "%" PRIi16, value);
}

[[nodiscard]] inline std::string_view operator>>(ReportQuery query,
                                                 uint16_t const& value) {
  STX_INTERNAL_MAKE_REPORT_(query.buffer, "%" PRIu16, value);
}

[[nodiscard]] inline std::string_view operator>>(ReportQuery query,
                                                 int32_t const& value) {
  STX_INTERNAL_MAKE_REPORT_(query.buffer, "%" PRIi32, value);
}

[[nodiscard]] inline std::string_view operator>>(ReportQuery query,
                                                 uint32_t const& value) {
  STX_INTERNAL_MAKE_REPORT_(query.buffer, "%" PRIu32, value);
}

#undef STX_INTERNAL_MAKE_REPORT_

[[nodiscard]] inline std::string_view operator>>(ReportQuery,
                                                 std::string_view const& str) {
  return str;
}

[[nodiscard]] inline std::string_view operator>>(ReportQuery,
                                                 std::string const& str) {
  return str;
}

STX_END_NAMESPACE
