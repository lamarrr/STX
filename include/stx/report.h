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
/// the error type of a `Result<T, E>`
/// the type contains a string interpretation/description of the error type
/// it should be initialized with ASCII-encoded characters, especially due to
/// cross-platform portability.
/// TODO: remove the null terminator
/// kMaxReportSize doesn't need it????
class [[nodiscard]] Report {
 public:
  using storage_type = std::array<char, kMaxReportSize>;

  [[nodiscard]] explicit constexpr Report(std::string_view info) noexcept
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

  [[nodiscard]] constexpr Report(Report const&) noexcept = default;
  [[nodiscard]] constexpr Report(Report &&) noexcept = default;
  constexpr Report& operator=(Report const&) noexcept = default;
  constexpr Report& operator=(Report&&) noexcept = default;
  constexpr ~Report() noexcept = default;

  [[nodiscard]] constexpr std::string_view const what() const noexcept {
    return std::string_view(report_.begin(), used_size_);
  };

  template <typename T>
  friend Report operator>>(ReportQuery, T const&) noexcept;

 private:
  storage_type report_;
  size_t used_size_;
};

// move definitions
// second argument to panic_handler
// this holds a reference to the report's data and is used accross
// ABI-boundaries as Report can vary accross configurations
class [[nodiscard]] ReportPayload {
 public:
  [[nodiscard]] explicit constexpr ReportPayload(Report const& report) noexcept
      : content_{report.what()} {}

  [[nodiscard]] constexpr ReportPayload(ReportPayload const&) noexcept =
      default;
  [[nodiscard]] constexpr ReportPayload(ReportPayload &&) noexcept = default;
  constexpr ReportPayload& operator=(ReportPayload const&) noexcept = default;
  constexpr ReportPayload& operator=(ReportPayload&&) noexcept = default;
  constexpr ~ReportPayload() noexcept = default;

  [[nodiscard]] std::string_view const& data() const noexcept {
    return content_;
  }

 private:
  std::string_view content_;
};

template <typename T>
concept Reportable = requires(ReportQuery const query, T const& v) {
  { query >> v }
  ->same_as<Report>;
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

};  // namespace internal

// consider integral concept

[[nodiscard]] inline Report operator>>(ReportQuery,
                                       exact<int8_t> auto const& v) noexcept {
  char buffer[5];
  auto written = std::snprintf(buffer, sizeof(buffer), "%" PRIi8, v);
  if (written == 0) internal::report::write_unknown(buffer);

  return Report(buffer);
}

[[nodiscard]] inline Report operator>>(ReportQuery,
                                       exact<uint8_t> auto const& v) noexcept {
  char buffer[4];
  auto written = std::snprintf(buffer, sizeof(buffer), "%" PRIu8, v);
  if (written == 0) internal::report::write_unknown(buffer);

  return Report(buffer);
}

[[nodiscard]] inline Report operator>>(ReportQuery,
                                       exact<int16_t> auto const& v) noexcept {
  char buffer[7];
  auto written = std::snprintf(buffer, sizeof(buffer), "%" PRId16, v);
  if (written == 0) internal::report::write_unknown(buffer);

  return Report(buffer);
}

[[nodiscard]] inline Report operator>>(ReportQuery,
                                       exact<uint16_t> auto const& v) noexcept {
  char buffer[7];
  auto written = std::snprintf(buffer, sizeof(buffer), "%" PRIu16, v);
  if (written == 0) internal::report::write_unknown(buffer);

  return Report(buffer);
}

[[nodiscard]] inline Report operator>>(ReportQuery,
                                       exact<int32_t> auto const& v) noexcept {
  char buffer[12];
  auto written = std::snprintf(buffer, sizeof(buffer), "%" PRId32, v);
  if (written == 0) internal::report::write_unknown(buffer);

  return Report(buffer);
}

[[nodiscard]] inline Report operator>>(ReportQuery,
                                       exact<uint32_t> auto const& v) noexcept {
  char buffer[11];
  auto written = std::snprintf(buffer, sizeof(buffer), "%" PRIu32, v);
  if (written == 0) internal::report::write_unknown(buffer);

  return Report(buffer);
}

[[nodiscard]] inline Report operator>>(
    ReportQuery, exact<std::string_view> auto const& v) noexcept {
  return Report(v);
}

[[nodiscard]] inline Report operator>>(
    ReportQuery, exact<std::string> auto const& v) noexcept {
  return Report(v);
}

};