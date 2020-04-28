#include <iostream>
#include <variant>

#include "benchmark/benchmark.h"
#include "stx/option.h"

enum Error { ZeroDivision, NoError };

using stx::Result, stx::Ok, stx::Err, stx::make_ok, stx::make_err;

std::variant<double, Error> variant_divide(double numerator,
                                           double denominator) {
  if (denominator == 0.0) return Error::ZeroDivision;

  return numerator / denominator;
}

double exception_divide(double numerator, double denominator) {
  if (denominator == 0.0) throw Error::ZeroDivision;

  return numerator / denominator;
}

Result<double, Error> result_divide(double numerator, double denominator) {
  if (denominator == 0.0) return Err(Error::ZeroDivision);
  return Ok(numerator / denominator);
}

Error c_style_divide(double num, double div, double* result) {
  if (div == 0.0) return Error::ZeroDivision;
  *result = num / div;
  return Error::NoError;
}

void Variant_HappyPath(benchmark::State& state) {  // NOLINT
  for (auto _ : state) {
    auto result = variant_divide(1.0, 0.5);
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
      auto value = exception_divide(1.0, 0.5);
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
    result_divide(1.0, 0.5).match(
        [](auto value) { benchmark::DoNotOptimize(value); },
        [](auto err) {
          if (err == Error::ZeroDivision) {
            benchmark::DoNotOptimize(err);
          }
        });
  }
}

void CStyle_HappyPath(benchmark::State& state) {  // NOLINT
  for (auto _ : state) {
    double result;
    auto err = c_style_divide(1.0, 0.5, &result);
    if (err == Error::ZeroDivision) {
      benchmark::DoNotOptimize(err);
    } else {
      benchmark::DoNotOptimize(result);
    }
  }
}

void Variant_SadPath(benchmark::State& state) {  // NOLINT
  for (auto _ : state) {
    auto result = variant_divide(1.0, 0.0);
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
      auto value = exception_divide(1.0, 0.0);
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
    result_divide(1.0, 0.0).match(
        [](auto value) { benchmark::DoNotOptimize(value); },
        [](auto err) {
          if (err == Error::ZeroDivision) {
            benchmark::DoNotOptimize(err);
          }
        });
  }
}

void CStyle_SadPath(benchmark::State& state) {  // NOLINT
  for (auto _ : state) {
    double result;
    auto err = c_style_divide(1.0, 0.0, &result);
    if (err == Error::ZeroDivision) {
      benchmark::DoNotOptimize(err);
    } else {
      benchmark::DoNotOptimize(result);
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
