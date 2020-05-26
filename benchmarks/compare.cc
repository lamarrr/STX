

#include <array>
#include <iostream>
#include <memory>
#include <numeric>
#include <optional>
#include <thread>
#include <vector>

#include "fmt/format.h"
#include "stx/option.h"
#include "stx/result.h"

using std::move, std::array, std::for_each, std::cout, std::accumulate,
    std::optional, std::unique_ptr, std::string_view;
using stx::make_none, stx::make_some;
using stx::Option, stx::Some, stx::None, stx::NoneType;
using stx::Result, stx::Ok, stx::Err;

auto fn_stx(Option<array<int, 5>>&& arr_a) noexcept -> Option<array<int, 5>> {
  auto arr = move(arr_a).unwrap_or({1, 5, 3, 4, 5});
  auto sum = accumulate(arr.begin(), arr.end(), 0);
  if (sum > 0) {
    return Some(move(arr));
  } else {
    return Some(array<int, 5>{1, 2, 3, 4, 5});
  }
}

auto fn_stm() -> Option<NoneType> { return Some(NoneType{}); }

auto fn_std(optional<array<int, 5>>&& arr_a) noexcept
    -> optional<array<int, 5>> {
  auto arr = arr_a.value_or(array<int, 5>{1, 5, 3, 4, 5});
  auto sum = accumulate(arr.begin(), arr.end(), 0);
  if (sum > 0) {
    return move(arr);
  } else {
    return array<int, 5>{1, 2, 3, 4, 5};
  }
}

auto fn_mem() -> optional<unique_ptr<int[]>> {
  auto f = std::make_unique<int[]>(1024);
  return f;
}

auto divide(double numerator, double denominator) -> Option<double> {
  if (denominator == 0.0) {
    return None;
  } else {
    return Some(numerator / denominator);
  }
}

int main() {
  fmt::print("Started\n");
  enum class Version { Version1 = 1, Version2 = 2 };

  auto parse_version =
      [](array<uint8_t, 5> const& header) -> Result<Version, string_view> {
    if (header.size() < 1) {
      return Err<string_view>("invalid header length");
    } else if (header[0] == 1) {
      return Ok(Version::Version1);
    } else if (header[0] == 2) {
      return Ok(Version::Version2);
    } else {
      return Err<string_view>("invalid version");
    }
  };

  parse_version({0x01u, 0x02u, 0x03u, 0x04u, 0x05u})
      .match(
          [](auto value) { fmt::print("Working with version: {}\n", value); },
          [](auto err) { fmt::print("Error parsing header: {}\n", err); });

  auto gg = array<int, 5>{1, 2, 3, 4, 5};
  auto arr = fn_stx(Some<array<int, 5>>({1, 2, 3, 4, 5}));

  auto g = fn_stm() == None;
  fmt::print("{}\n", g);

  move(arr).match([](auto value) { fmt::print("{}\n", value[0]); },
                  []() { fmt::print("has no value\n"); });

  if (arr == Some(&gg)) {
    fmt::print("Same\n");
  }

  // The return value of the function is an option
  auto result = divide(2.0, 3.0);

  // to retrieve the value
  if (result.is_some()) {
    fmt::print("{}\n", move(result).unwrap());
  } else {
    fmt::print("has no value\n");
  }

  move(result).match([](auto value) { fmt::print("{}\n", value); },
                     []() { fmt::print("has no value"); });

  using namespace std::string_literals;

  using std::string, std::string_view;

  enum class Error { InvalidEmail };
  auto email = "EMAIL: 'Hello World!'";

  auto check_email = [](string_view s) -> Result<string_view, Error> {
    if (!s.starts_with("EMAIL: ")) return Err(Error::InvalidEmail);
    return Ok(move(s));
  };

  auto msg = check_email(email).map([](auto s) { return string(s.substr(7)); });

  auto y = make_none<int>().match(
      [](auto) -> int { return 200; },
      []() -> int { stx::panic("Let the world collapse!"); });
  (void)y;
}
