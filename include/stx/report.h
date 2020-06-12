/**
 * @file report.h
 * @author Basit Ayantunde <rlamarrr@gmail.com>
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

#include <cinttypes>
#include <cstddef>
#include <cstdio>
#include <string>
#include <string_view>

#include "stx/common.h"

STX_BEGIN_NAMESPACE

#ifndef STX_REPORT_RESERVE_SIZE
constexpr size_t kReportReserveSize = 128;
#else
constexpr size_t kReportReserveSize = STX_REPORT_RESERVE_SIZE;
#endif

constexpr char const kReportTruncationMessage[] =
    "... (error report truncated)";

// this also ensures we don't perform an out-of-bounds access on the reserve
static_assert(kReportReserveSize > (sizeof(kReportTruncationMessage) - 1),
              "STX_REPORT_RESERVE_SIZE must contain truncation message");

/// Tag type for dispatching reports
struct ReportQuery {};

/// Tag value for dispatching reports
constexpr ReportQuery const report_query{};

// rather than having one report type and paying at runtime by having a `bool`
// to tell if the report string is allocated on the stack or is a forwarding
// reference we pay for it at compile-time by tag dispatch

/// `FixedReport` is an error-type with a fixed-size stack-allocated buffer,
/// containing an error string denoting the cause of the error from
/// the error type of a `Result<T, E>`.
///
/// The error-string stored in the report is truncated (and contains a message
/// to signify truncation) if its given string is longer than
/// `stx::kReportReserveSize`.
///
/// # When to use `FixedReport` against `SpanReport`
///
/// - When the error types don't contain the string description of the errors
/// themselves and the description strings are not `static`
/// - When the string-description is not known at compile-time
///
///
/// # Constructing a `FixedReport` for your error type
///
/// `FixedReport`s are automatically constructed for `uint8_t`, `uint16_t`,
/// `uint32_t`,`int8_t`, `int16_t`, `int32_t`, `string`, `string_view`, and
/// pointers, as the error strings are not known at compile-time and neither are
/// the types containing a description of the error.
///
/// NOTE: If the error type already provides an error message or the needed
/// error-message for the type is a `static` duration value and known at
/// compile, you can use `SpanReport` instead which just forwards a
/// **reference** to the error string.
///
///
/// ## Defining for your error type
///
/// ```
///
/// // assuming the error type contains only an int, and no string description.
/// struct Error {
///
///     // error integer
///     int error;
///
///     // formats error message
///     void format_into(const char*, size_t size) const noexcept;
///
/// };
///
///
/// inline FixedReport operator>>(ReportQuery, Error const& err) noexcept {
///    char buffer[64];
///    err.format_into(buffer, 64);
///    return FixedReport(buffer, 64);
///
/// };
///
/// ```
///
/// # Exception-safety
///
/// Ensure to not throw exceptions.
///
// rather than having a `bool` to tell if this is allocated on the stack or is a
// forwarding reference we pay for it at compile time by tag dispatch
struct [[nodiscard]] FixedReport {
  using storage_type = char[kReportReserveSize];

  constexpr FixedReport() noexcept : reserve_{}, report_size_{0} {}

  /// size: string size excluding null-terminator
  constexpr FixedReport(const char* str, size_t size) noexcept
      : reserve_{}, report_size_{size} {
    *this = FixedReport(std::string_view(str, size));
  }

  explicit constexpr FixedReport(std::string_view const& info) noexcept
      : reserve_{}, report_size_{0} {
    // account for truncation
    // check bounds
    size_t const str_size = info.size();
    bool const should_truncate = str_size > kReportReserveSize;
    size_t const str_clip_size =
        should_truncate
            ? (kReportReserveSize - (sizeof(kReportTruncationMessage) - 1))
            : str_size;

    size_t pos = 0;
    for (; pos < str_clip_size; pos++) {
      reserve_[pos] = info[pos];
    }

    if (should_truncate) {
      for (; pos < kReportReserveSize; pos++) {
        reserve_[pos] = kReportTruncationMessage[pos - str_clip_size];
      }
    }

    report_size_ = pos;
  }

  constexpr FixedReport(FixedReport const&) noexcept = default;
  constexpr FixedReport(FixedReport &&) noexcept = default;
  constexpr FixedReport& operator=(FixedReport const&) noexcept = default;
  constexpr FixedReport& operator=(FixedReport&&) noexcept = default;
  ~FixedReport() noexcept = default;

  [[nodiscard]] constexpr std::string_view what() const noexcept {
    return std::string_view(reserve_, report_size_);
  };

 private:
  storage_type reserve_;
  size_t report_size_;
};

/// `SpanReport` is an error-report type containing a reference to an
/// error-string stating the cause of the error from the error type of a
/// `Result<T, E>`.
///
///
/// # When to use `SpanReport` against `FixedReport`
///
/// - When the error types contain the string description of the errors
/// themselves and you can get a reference to the error.
///
/// **TIP**: `SpanReport` is named after `std::span` as it is only a type-erased
/// reference-view of the error string.
///
/// # Constructing a `SpanReport` for your error type
///
///
/// **NOTE**: character literals (i.e. ```"A STRING"```) have static storage
/// duration and live throughout the program, constructing a `SpanReport` with
/// them just forwards a reference to them, it is safe to do this)
///
/// ``` cpp
///
/// namespace net{
///
///   enum class Error {
///       ConnectionError,
///       ServerClosed,
///       ServerUnreachable
///   };
///
/// };
///
/// SpanReport operator>>(ReportQuery, net::Error const& err) noexcept {
///   switch(err){
///          case net::Error::ConnectionError:
///            return SpanReport("Connection error occurred");
///          case net::Error::ServerClosed:
///            return SpanReport("Server closed unexpectedly");
///          case net::Error::ServerUnreachable:
///            return SpanReport("Server was unreachable");
///    }
///  }
///
///
/// ```
///
///
/// ``` cpp
///
/// namespace net{
///
///      class Error {
///
///           // returns a reference to the error string
///           std::string const & what() const noexcept;
///
///
///            // returns a string_view to the error string
///           std::string_view better_what() const noexcept;
///
///
///            // returns a const string_view reference of the error
///           std::string_view const& best_what() const noexcept;
///
///
///           // `dangerous_what` creates a new string and returns it, totally
///           // dangerous as a constructor for `SpanReport` because
///           // `SpanReport` binds a reference to its constructor argument
///           // which in this case is a temporary and will outlive the
///           // `SpanReport` if the `SpanReport` is returned from the report
///           query
///           // below.
///           //
///           std::string dangerous_what() const noexcept;
///       };
///
/// };
///
///
///
/// SpanReport operator>>(ReportQuery, net::Error const& err) noexcept {
///   return SpanReport(err.what());
/// }
///
/// ```
///
/// # Exception-safety
///
/// Ensure to not throw exceptions.
///
struct [[nodiscard]] SpanReport {
  // maintains a reference to a string, instead of using a stack
  // allocated reservation buffer
  // it does not own the contents, so make sure the referenced buffer lives
  // longer than this object

  constexpr SpanReport() noexcept : custom_payload_{nullptr}, report_size_{0} {}

  /// size: string size excluding null-terminator
  constexpr SpanReport(const char* str, size_t size) noexcept
      : custom_payload_{str}, report_size_{size} {}

  explicit constexpr SpanReport(std::string_view const& str) noexcept
      : custom_payload_{str.data()}, report_size_{str.size()} {}

  [[nodiscard]] constexpr std::string_view what() const noexcept {
    return std::string_view(custom_payload_, report_size_);
  };

  constexpr SpanReport(SpanReport const&) noexcept = default;
  constexpr SpanReport(SpanReport &&) noexcept = default;
  constexpr SpanReport& operator=(SpanReport const&) noexcept = default;
  constexpr SpanReport& operator=(SpanReport&&) noexcept = default;
  ~SpanReport() noexcept = default;

 private:
  const char* custom_payload_;
  size_t report_size_;
};

/// `ReportPayload` holds a reference to the report's data and is used accross
/// ABI-boundaries by panic handlers, as `FixedReport` can vary accross
/// configurations. `ReportPayload` is essentially a type-erased view of
/// `FixedReport` and `SpanReport` (as in `std::span`).
///
/// `ReportPayload` is the type of the second argument to
/// `panic_handler`.
///
class [[nodiscard]] ReportPayload {
 public:
  explicit constexpr ReportPayload(FixedReport const& report) noexcept
      : content_{report.what()} {}

  explicit constexpr ReportPayload(SpanReport const& report) noexcept
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

// this allows tolerance for platforms without snprintf.
constexpr char const kFormatError[] = "<format error>";
constexpr size_t const kFormatErrorSize = sizeof(kFormatError) - 1;

template <typename T>
[[nodiscard]] inline SpanReport operator>>(ReportQuery, T const&) noexcept {
  return SpanReport();
}

#define STX_INTERNAL_MAKE_REPORT_(STX_ARG_SIZE, STX_ARG_FORMAT, STX_ARG_VALUE) \
  /* string size + terminating null character */                               \
  char fmt_buffer[STX_ARG_SIZE + 1];                                           \
  int fmt_buffer_size = std::snprintf(fmt_buffer, STX_ARG_SIZE + 1,            \
                                      STX_ARG_FORMAT, STX_ARG_VALUE);          \
  if (fmt_buffer_size < 0 || fmt_buffer_size >= STX_ARG_SIZE + 1) {            \
    return FixedReport(kFormatError, kFormatErrorSize);                        \
  } else {                                                                     \
    return FixedReport(fmt_buffer, fmt_buffer_size);                           \
  }

template <typename T>
[[nodiscard]] inline FixedReport operator>>(ReportQuery,
                                            T const* const& ptr) noexcept {
  STX_INTERNAL_MAKE_REPORT_(internal::kxPtrFmtSize, "0x%" PRIxPTR,
                            reinterpret_cast<uintptr_t const>(ptr));
}

template <typename T>
[[nodiscard]] inline FixedReport operator>>(ReportQuery,
                                            T* const& ptr) noexcept {
  STX_INTERNAL_MAKE_REPORT_(internal::kxPtrFmtSize, "0x%" PRIxPTR,
                            reinterpret_cast<uintptr_t>(ptr));
}

[[nodiscard]] inline FixedReport operator>>(ReportQuery,
                                            int8_t const& v) noexcept {
  STX_INTERNAL_MAKE_REPORT_(internal::kI8FmtSize, "%" PRIi8, v);
}

[[nodiscard]] inline FixedReport operator>>(ReportQuery,
                                            uint8_t const& v) noexcept {
  STX_INTERNAL_MAKE_REPORT_(internal::kU8FmtSize, "%" PRIu8, v);
}

[[nodiscard]] inline FixedReport operator>>(ReportQuery,
                                            int16_t const& v) noexcept {
  STX_INTERNAL_MAKE_REPORT_(internal::kI16FmtSize, "%" PRIi16, v);
}

[[nodiscard]] inline FixedReport operator>>(ReportQuery,
                                            uint16_t const& v) noexcept {
  STX_INTERNAL_MAKE_REPORT_(internal::kU16FmtSize, "%" PRIu16, v);
}

[[nodiscard]] inline FixedReport operator>>(ReportQuery,
                                            int32_t const& v) noexcept {
  STX_INTERNAL_MAKE_REPORT_(internal::kI32FmtSize, "%" PRIi32, v);
}

[[nodiscard]] inline FixedReport operator>>(ReportQuery,
                                            uint32_t const& v) noexcept {
  STX_INTERNAL_MAKE_REPORT_(internal::kU32FmtSize, "%" PRIu32, v);
}

#undef STX_INTERNAL_MAKE_REPORT_

[[nodiscard]] inline SpanReport operator>>(ReportQuery,
                                           std::string_view const& v) noexcept {
  return SpanReport(v);
}

[[nodiscard]] inline SpanReport operator>>(ReportQuery,
                                           std::string const& v) noexcept {
  return SpanReport(v);
}

STX_END_NAMESPACE
