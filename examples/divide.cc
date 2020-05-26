#include <iostream>

#include "stx/option.h"

using std::cout, std::endl;
using stx::Option, stx::Some, stx::None;

constexpr auto divide(double numerator, double denominator) -> Option<double> {
  if (denominator == 0.0) return None;
  return Some(numerator / denominator);
}

constexpr auto x = divide(1.0, 0.0);
static_assert(x == None);

constexpr auto y = divide(1.0, 0.2);
static_assert(y == Some(5.0));
static_assert(y.clone().map([](auto x) { return x * 10.0; }).unwrap_or(-10.0) ==
              50.0);

int main() {
  divide(5.0, 2.0).match(
      [](auto value) { cout << "Result: " << value << endl; },
      []() { cout << "Cannot divide by zero" << endl; });
}
