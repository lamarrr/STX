#include <iostream>
#include <variant>

#include "benchmark/benchmark.h"
#include "stx/option.h"

enum Error { ZeroDivision, NoError };

using std::variant, std::move;
using stx::Result, stx::Ok, stx::Err, stx::make_ok, stx::make_err;

auto variant_divide(double num, double div) -> variant<double, Error> {
  if (div == 0.0) return Error::ZeroDivision;
  return num / div;
}

inline variant<double, Error> divide_by_variant(double num,
                                                variant<double, Error>&& div) {
  if (std::holds_alternative<double>(div)) {
    auto d = std::get<double>(div);
    if (d == 0.0) return Error::ZeroDivision;
    return num / d;
  } else {
    return std::get<Error>(div);
  }
}

// no need to rewrite another one since the exception would already be caught?
double exception_divide(double numerator, double denominator) {
  if (denominator == 0.0) throw Error::ZeroDivision;
  return numerator / denominator;
}

inline double divide_by_exceptional(double num, double div) {
  if (div == 0.0) throw Error::ZeroDivision;
  return num / div;
}

Result<double, Error> result_divide(double numerator, double&& denominator) {
  if (denominator == 0.0) return Err(Error::ZeroDivision);
  return Ok(numerator / denominator);
}

inline auto divide_by_result(double num, Result<double, Error>&& div)
    -> Result<double, Error> {
  return move(div).match(
      [&](auto d) -> Result<double, Error> {
        if (d == 0.0) return Err(Error::ZeroDivision);
        return Ok(num / d);
      },
      [](auto e) -> Result<double, Error> { return Err(move(e)); });
}

Error c_style_divide(double num, double div, double* result) {
  if (div == 0.0) return Error::ZeroDivision;
  *result = num / div;
  return Error::NoError;
}

inline Error divide_by_c_style(double num, double div, double* result) {
  if (div == 0.0) return Error::ZeroDivision;
  *result = num / div;
  return Error::NoError;
}

void Variant_HappyPath(benchmark::State& state) {  // NOLINT
  for (auto _ : state) {
    auto result = divide_by_variant(5.0, variant_divide(0.444, 0.5));

    if (std::holds_alternative<double>(result)) {
      auto value = std::get<double>(result);
      benchmark::DoNotOptimize(value);
    } else {
      auto err = std::get<Error>(result);
      if (err == Error::ZeroDivision) {
        benchmark::DoNotOptimize(err);
      }
    }
  }
}

void Exception_HappyPath(benchmark::State& state) {  // NOLINT
  for (auto _ : state) {
    try {
      // either of the two functions can throw an exception, we thus catch the
      // error with a single catch block
      auto value = divide_by_exceptional(5.0, exception_divide(0.444, 0.5));
      benchmark::DoNotOptimize(value);
    } catch (Error const& err) {
      if (err == Error::ZeroDivision) {
        benchmark::DoNotOptimize(err);
      }
    }
  }
}

void Result_HappyPath(benchmark::State& state) {  // NOLINT
  for (auto _ : state) {
    divide_by_result(5.0, result_divide(0.444, 0.5))
        .match([](auto v) { benchmark::DoNotOptimize(v); },
               [](auto e) {
                 if (e == Error::ZeroDivision) benchmark::DoNotOptimize(e);
               });
  }
}

void CStyle_HappyPath(benchmark::State& state) {  // NOLINT
  for (auto _ : state) {
    double result;
    auto err = c_style_divide(0.444, 0.5, &result);
    if (err == Error::ZeroDivision) {
    } else {
      auto err = divide_by_c_style(5.0, result, &result);
      if (err == Error::ZeroDivision) {
        benchmark::DoNotOptimize(err);
      } else {
        benchmark::DoNotOptimize(result);
      }
    }
  }
}

void Variant_SadPath(benchmark::State& state) {  // NOLINT
  for (auto _ : state) {
    auto result = divide_by_variant(5.0, variant_divide(0.0, 0.5));

    if (std::holds_alternative<double>(result)) {
      auto value = std::get<double>(result);
      benchmark::DoNotOptimize(value);
    } else {
      auto err = std::get<Error>(result);
      if (err == Error::ZeroDivision) {
        benchmark::DoNotOptimize(err);
      }
    }
  }
}

void Exception_SadPath(benchmark::State& state) {  // NOLINT
  for (auto _ : state) {
    try {
      // either of the two functions can throw an exception, we thus catch the
      // error with a single catch block
      auto value = divide_by_exceptional(5.0, exception_divide(0.0, 0.5));
      benchmark::DoNotOptimize(value);
    } catch (Error const& err) {
      if (err == Error::ZeroDivision) {
        benchmark::DoNotOptimize(err);
      }
    }
  }
}

void Result_SadPath(benchmark::State& state) {  // NOLINT
  for (auto _ : state) {
    divide_by_result(5.0, result_divide(0.0, 0.5))
        .match([](auto v) { benchmark::DoNotOptimize(v); },
               [](auto e) {
                 if (e == Error::ZeroDivision) benchmark::DoNotOptimize(e);
               });
  }
}

void CStyle_SadPath(benchmark::State& state) {  // NOLINT
  for (auto _ : state) {
    double result;
    auto err = c_style_divide(0.0, 0.5, &result);
    if (err == Error::ZeroDivision) {
    } else {
      auto err = divide_by_c_style(5.0, result, &result);
      if (err == Error::ZeroDivision) {
        benchmark::DoNotOptimize(err);
      } else {
        benchmark::DoNotOptimize(result);
      }
    }
  }
}

BENCHMARK(Variant_HappyPath);
BENCHMARK(Exception_HappyPath);
BENCHMARK(Result_HappyPath);
BENCHMARK(CStyle_HappyPath);

BENCHMARK(Variant_SadPath);
BENCHMARK(Exception_SadPath);
BENCHMARK(Result_SadPath);
BENCHMARK(CStyle_SadPath);
