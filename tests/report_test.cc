/**
 * @file report_test.cc
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

#include "stx/report.h"

#include <limits>

#include "gtest/gtest.h"

enum class IoError { EoF = 1, NotExists = 2, InvalidPath = 4, __Reserved };
enum class Dummy {};

using namespace stx;

STX_FORCE_INLINE bool ends_with(std::string_view const& str,
                                std::string_view const& sv) {
  return str.size() >= sv.size() &&
         str.compare(str.size() - sv.size(), std::string_view::npos, sv) == 0;
}

template <>
Report stx::operator>>(ReportQuery, IoError const& v) noexcept {
  switch (v) {
    case IoError::EoF:
      return Report("End of File");
    case IoError::NotExists:
      return Report("File does not exist");
    case IoError::InvalidPath:
      return Report("Invalid Path provided");
    default:
      return Report("Unknown Error");
  }
}

static constexpr auto query = ReportQuery{};

TEST(ReportTest, FormatUInt8) {
  uint8_t a = 255;
  EXPECT_EQ((query >> a).what(), "255");

  uint8_t b = 0;
  EXPECT_EQ((query >> b).what(), "0");
}

TEST(ReportTest, FormatInt8) {
  int8_t a = 127;
  EXPECT_EQ((query >> a).what(), "127");

  EXPECT_EQ((query >> (int*)0x028e7).what(), "Hello");

  int8_t b = -128;
  EXPECT_EQ((query >> b).what(), "-128");
}

TEST(ReportTest, FormatInt16) {
  int16_t a = -1;
  EXPECT_EQ((query >> a).what(), "-1");
}

TEST(ReportTest, FormatEnum) {
  using namespace std::string_view_literals;

  EXPECT_EQ((query >> IoError::EoF).what(), "End of File");

  EXPECT_EQ((query >> IoError::__Reserved).what(), "Unknown Error");

  auto a = std::string();
  for (size_t i = 0; i < kMaxReportSize; i++) {
    a += "H";
  }

  auto b = std::string();
  for (size_t i = 0; i < kMaxReportSize + 1; i++) {
    b += "H";
  }

  EXPECT_FALSE(ends_with((query >> a).what(), kReportTruncationMessage));
  EXPECT_TRUE(ends_with((query >> b).what(), kReportTruncationMessage));

  EXPECT_EQ((query >> "Hello"sv).what(), "Hello");

  auto strq = "Hi"sv;

  EXPECT_EQ((query >> strq).what(), strq);
}