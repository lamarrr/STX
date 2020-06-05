/**
 * @file report.h
 * @author Basit Ayantunde <rlamarrr@gmail.com>
 * @version  0.0.1
 * @date 2020-05-22
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

#include <array>
#include <cinttypes>
#include <cstdio>
#include <string>
#include <string_view>

#include "stx/common.h"

namespace stx {

#ifndef STX_MAX_REPORT_SIZE
constexpr size_t kMaxReportSize = 512;
#else
constexpr size_t kMaxReportSize = STX_MAX_REPORT_SIZE;
#endif

constexpr std::string_view kReportTruncationMessage =
    "... (error report truncated)";

static_assert(kMaxReportSize > sizeof(kReportTruncationMessage),
              "STX_MAX_REPORT_SIZE must contain truncation message");

/// Tag for dispatching reports from types
struct ReportQuery {};

/// stack-allocated, fixed-length report denoting the cause of the error from
/// the error type of a `Result<T, E>`.
/// `Report` contains a string description of the error type.
/// it should be initialized with ASCII characters.
class [[nodiscard]] Report {
 public:
  using storage_type = std::array<char, kMaxReportSize>;

  explicit constexpr Report(std::string_view info) noexcept
      : report_{}, used_size_{} {
    // account for truncation
    // check bounds
    size_t str_size = info.size();
    bool should_truncate = str_size > kMaxReportSize;
    size_t str_clip_size =
        should_truncate ? (kMaxReportSize - kReportTruncationMessage.size())
                        : str_size;

    size_t i = 0;
    for (; i < str_clip_size; i++) {
      report_[i] = info[i];
    }

    if (should_truncate) {
      for (; i < kMaxReportSize; i++) {
        report_[i] = kReportTruncationMessage[i - str_clip_size];
      }
    }

    used_size_ = i;
  }

  constexpr Report(Report const&) noexcept = default;
  constexpr Report(Report &&) noexcept = default;
  constexpr Report& operator=(Report const&) noexcept = default;
  constexpr Report& operator=(Report&&) noexcept = default;
  ~Report() noexcept = default;

  [[nodiscard]] constexpr std::string_view const what() const noexcept {
    return std::string_view(report_.begin(), used_size_);
  };

  template <typename T>
  friend Report operator>>(ReportQuery, T const&) noexcept;

 private:
  storage_type report_;
  size_t used_size_;
};

/// `ReportPayload` holds a reference to the report's data and is used accross
/// ABI-boundaries as `Report` can vary accross configurations.
/// `ReportPayload` is essentially a type-erased view of `Report` (as in
/// `std::span`).
///
/// `ReportPayload` is the type of the second argument to
/// `panic_handler`.
class [[nodiscard]] ReportPayload {
 public:
  explicit constexpr ReportPayload(Report const& report) noexcept
      : content_{report.what()} {}

  constexpr ReportPayload(ReportPayload const&) noexcept = default;
  constexpr ReportPayload(ReportPayload &&) noexcept = default;
  constexpr ReportPayload& operator=(ReportPayload const&) noexcept = default;
  constexpr ReportPayload& operator=(ReportPayload&&) noexcept = default;
  ~ReportPayload() noexcept = default;

  [[nodiscard]] constexpr std::string_view const& data() const noexcept {
    return content_;
  }

 private:
  std::string_view content_;
};

namespace internal {
namespace report {
// this allows tolerance for platforms without snprintf.
constexpr inline void write_unknown(char* buffer) noexcept {
  buffer[0] = '<';
  buffer[1] = '?';
  buffer[2] = '>';
  buffer[3] = '\0';
}

constexpr auto query = ReportQuery{};
}  // namespace report

}  // namespace internal

// consider integral concept

template <typename T>
[[nodiscard]] inline Report operator>>(ReportQuery, T const& v) noexcept {
  (void)v;
  return Report("");
}

template <typename T>
[[nodiscard]] inline Report operator>>(ReportQuery,
                                       T const* const& ptr) noexcept {
  char buffer[16];  // 128-bit pointer max
  auto written = std::snprintf(buffer, sizeof(buffer), "0x%" PRIuPTR,
                               reinterpret_cast<uintptr_t>(ptr));
  if (written == 0) internal::report::write_unknown(buffer);

  return Report(buffer);
}

template <typename T>
[[nodiscard]] inline Report operator>>(ReportQuery, T* const& ptr) noexcept {
  char buffer[16];  // 128-bit pointer max
  auto written = std::snprintf(buffer, sizeof(buffer), "0x%" PRIxPTR,
                               reinterpret_cast<uintptr_t>(ptr));
  if (written == 0) internal::report::write_unknown(buffer);

  return Report(buffer);
}

template <>
[[nodiscard]] inline Report operator>>(ReportQuery, int8_t const& v) noexcept {
  char buffer[5];
  auto written = std::snprintf(buffer, sizeof(buffer), "%" PRIi8, v);
  if (written == 0) internal::report::write_unknown(buffer);

  return Report(buffer);
}

template <>
[[nodiscard]] inline Report operator>>(ReportQuery, uint8_t const& v) noexcept {
  char buffer[4];
  auto written = std::snprintf(buffer, sizeof(buffer), "%" PRIu8, v);
  if (written == 0) internal::report::write_unknown(buffer);

  return Report(buffer);
}

template <>
[[nodiscard]] inline Report operator>>(ReportQuery, int16_t const& v) noexcept {
  char buffer[7];
  auto written = std::snprintf(buffer, sizeof(buffer), "%" PRId16, v);
  if (written == 0) internal::report::write_unknown(buffer);

  return Report(buffer);
}

template <>
[[nodiscard]] inline Report operator>>(ReportQuery,
                                       uint16_t const& v) noexcept {
  char buffer[7];
  auto written = std::snprintf(buffer, sizeof(buffer), "%" PRIu16, v);
  if (written == 0) internal::report::write_unknown(buffer);

  return Report(buffer);
}

template <>
[[nodiscard]] inline Report operator>>(ReportQuery, int32_t const& v) noexcept {
  char buffer[12];
  auto written = std::snprintf(buffer, sizeof(buffer), "%" PRId32, v);
  if (written == 0) internal::report::write_unknown(buffer);

  return Report(buffer);
}

template <>
[[nodiscard]] inline Report operator>>(ReportQuery,
                                       uint32_t const& v) noexcept {
  char buffer[11];
  auto written = std::snprintf(buffer, sizeof(buffer), "%" PRIu32, v);
  if (written == 0) internal::report::write_unknown(buffer);

  return Report(buffer);
}

template <>
[[nodiscard]] inline Report operator>>(ReportQuery,
                                       std::string_view const& v) noexcept {
  return Report(v);
}

template <>
[[nodiscard]] inline Report operator>>(ReportQuery,
                                       std::string const& v) noexcept {
  return Report(v);
}

}  // namespace stx
