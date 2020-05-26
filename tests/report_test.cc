#include "stx/report.h"

#include <limits>

#include "gtest/gtest.h"

enum class IoError { EoF = 1, NotExists = 2, InvalidPath = 4, __Reserved };
enum class Dummy {};

using namespace stx;

Report operator>>(ReportQuery, exact<IoError> auto const& v) {
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

static_assert(Reportable<int8_t>);
static_assert(Reportable<uint8_t>);
static_assert(Reportable<int16_t>);
static_assert(Reportable<uint16_t>);

static_assert(Reportable<IoError>);
static_assert(!Reportable<Dummy>);

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

  EXPECT_FALSE((query >> a).what().ends_with(kReportTruncationMessage));
  EXPECT_TRUE((query >> b).what().ends_with(kReportTruncationMessage));

  EXPECT_EQ((query >> "Hello"sv).what(), "Hello");

  auto strq = "Hi"sv;

  EXPECT_EQ((query >> strq).what(), strq);
}