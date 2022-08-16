/**
 * @file report_test.cc
 * @author Basit Ayantunde <rlamarrr@gmail.com>
 * @date 2020-06-05
 *
 * @copyright MIT License
 *
 * Copyright (c) 2020-2022 Basit Ayantunde
 *
 */

#include "stx/panic/report.h"

#include <limits>
#include <sstream>

#include "gtest/gtest.h"

using namespace stx;
using namespace std::literals;

// https://en.cppreference.com/w/cpp/string/basic_string_view/ends_with
STX_FORCE_INLINE bool ends_with(std::string_view const& str,
                                std::string_view const& sv) {
  return str.size() >= sv.size() &&
         str.compare(str.size() - sv.size(), std::string_view::npos, sv) == 0;
}

template <typename T>
std::string to_string(T const& value) {
  std::ostringstream stream;
  stream << value;
  return stream.str();
}

enum class IoError { EoF = 1, NotExists = 2, InvalidPath = 4, __Reserved };
enum class Dummy {};

std::string_view operator>>(ReportQuery, IoError const& v) {
  switch (v) {
    case IoError::EoF:
      return "End of File";
    case IoError::NotExists:
      return "File does not exist";
    case IoError::InvalidPath:
      return "Invalid Path provided";
    default:
      return "Unknown Error";
  }
}

char fmt_buffer[1024];

static constexpr auto query =
    ReportQuery{ReportQuery::Buffer{fmt_buffer, std::size(fmt_buffer)}};

TEST(ReportTest, FormatPointer) {
  EXPECT_TRUE((query >> (int*)0x28e7) == "0x28e7"s);

  EXPECT_TRUE((query >> (int const*)0x28e7) == "0x28e7"s);

  int const* const c = (int const*)0x28e7;
  EXPECT_TRUE((query >> c) == "0x28e7"s);
}

TEST(ReportTest, FormatUInt8) {
  uint8_t a = 255;
  EXPECT_EQ(query >> a, "255");

  uint8_t b = 0;
  EXPECT_EQ(query >> b, "0");
}

TEST(ReportTest, FormatInt8) {
  int8_t a = 127;
  EXPECT_EQ(query >> a, "127");

  int8_t b = -128;
  EXPECT_EQ(query >> b, "-128");
}

TEST(ReportTest, FormatUInt16) {
  uint16_t a = std::numeric_limits<uint16_t>::min();
  EXPECT_EQ(query >> a, to_string(a));

  uint16_t b = std::numeric_limits<uint16_t>::max();
  EXPECT_EQ(query >> b, to_string(b));
}

TEST(ReportTest, FormatInt16) {
  int16_t a = std::numeric_limits<int16_t>::min();
  EXPECT_EQ(query >> a, to_string(a));

  int16_t b = std::numeric_limits<int16_t>::max();
  EXPECT_EQ(query >> b, to_string(b));
}

TEST(ReportTest, FormatUInt32) {
  uint32_t a = std::numeric_limits<uint32_t>::min();
  EXPECT_EQ(query >> a, to_string(a));

  uint32_t b = std::numeric_limits<uint32_t>::max();
  EXPECT_EQ(query >> b, to_string(b));
}

TEST(ReportTest, FormatInt32) {
  int32_t a = std::numeric_limits<int32_t>::min();
  EXPECT_EQ(query >> a, to_string(a));

  int32_t b = std::numeric_limits<int32_t>::max();
  EXPECT_EQ(query >> b, to_string(b));
}

TEST(ReportTest, FormatEnum) {
  using namespace std::string_view_literals;

  EXPECT_EQ(query >> IoError::EoF, "End of File");

  EXPECT_EQ(query >> IoError::__Reserved, "Unknown Error");
}

TEST(ReportTest, UnReportable) {
  struct Dummy {};
  EXPECT_EQ(query >> Dummy{}, "");
  int c = 98;
  stx::Ref<int> g(c);
  EXPECT_EQ(g, c);
}
