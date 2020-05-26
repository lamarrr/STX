# STX

C++ 20 Error Handling and Monadic extensions to the C++ standard library

[***READ THE DOCUMENTATION***](http://lamarrr.github.io/blog/error-handling)

## Overview

## Important Note

- Some methods like .match, .map, .unwrap ***consume*** the stored value
and thus the Result or Option has to be destroyed immediately. Example:

```cpp

auto get_socket()->Result<int, int>


```

- The object is not to be used after a std::move! (though in a valid state)

## Features

- Provides a `Result<T, E>` and `Option<T>` monad implementation
- A unified error handling and reporting
- detachable panicking or error handling mechanisms
- Suitable and easily adaptable for embedded systems
- easy debugging

## Design Goals (Features)

- `Result<T, E>` and `Option<T>` as perfect forwarding return types (explicit zero-copy guarantee).
- Monads to make ...
- Make edge cases obvious
- Proper documentation
- Bring sanity to error handling
- explicitly communicated lifetime rules
- provide easier alternatives
- avoid `SFINAE` and other subtleties
- high performance with little binary and memory footprint
- implicit conversions that can easily occur with std::variant
- eliminate repitive code and abstractable error handling logic

## Similar Implementations / Attempts

- Pigweed `Status`, `StatusCode` and `Result`
- Fuchsia `Status` and `Result`
- Mozilla `Result<T, E>`
- Tensorflow's CHECK_OK, ENSURE_OK
- xpc_error_code
- `std::optional` and `std::variant`
- `tl::optional`
- [simd-json's simdjson_result and error_codes](https://github.com/simdjson/simdjson/blob/master/include/simdjson/inline/error.h)

## Basic Examples

### Option

```cpp

#include <iostream>

#include "stx/option.h"

using stx::Option, stx::Some, stx::None;

auto divide(double numerator, double denominator) -> Option<double> {
  if (denominator == 0.0) return None;
  return Some(numerator / denominator);
}

int main() {
  divide(5.0, 2.0).match(
      [](auto value) { std::cout << "Result: " << value << std::endl; },
      []() { std::cout << "Cannot divide by zero" << std::endl; });
}

```

### Result

```cpp

#include <array>
#include <iostream>

#include "stx/result.h"

using std::array, std::string_view;
using stx::Result, stx::Ok, stx::Err;
using namespace std::literals;

enum class Version { Version1 = 1, Version2 = 2 };

Result<Version, string_view> parse_version(array<uint8_t, 6> const& header) {
  switch (header.at(0)) {
    case 1:
      return Ok(Version::Version1);
    case 2:
      return Ok(Version::Version2);
    default:
      return Err("Unknown Version"sv);
  }
}

int main() {
  auto version =
      parse_version({2, 3, 4, 5, 6, 7}).unwrap_or("<Unknown Version>");

  std::cout << version << std::endl;
}

```

## Configuration Options

## Usage Notes

- The panic behavior should only be defined once per executable.
- The panic behavior should only be overriden in an executable project and not a library.
- Overriding panicking behavior:
  - In your project's CMake lists file set the `STX_OVERRIDE_PANIC_BEHAVIOUR` option to `ON`
  - In a source file add the following with your desired panic behaviour:

```cpp

  #include <cstddef>
  #include <string_view>

  #include "stx/panic.h"

  namespace stx {
  [[noreturn]] void panic_handler(std::string_view info,
                                  stx::SourceLocation source_location) {
    // do whatever you want here

    // the program must not continue
    std::abort();
  }
  };  // namespace stx

```

- Must specify C++20!!! source loc and concepts

### `Option<T>` methods

- `is_some`
- `is_none`
- `contains`
- `as_const_ref`
- `as_mut_ref`
- `expect`
- `unwrap`
- `unwrap_or`
- `unwrap_or_else`
- `map`
- `map_or`
- `map_or_else`
- `ok_or`
- `ok_or_else`
- `AND`
- `and_then`
- `filter`
- `OR`
- `or_else`
- `XOR`
- `take`
- `replace`
- `clone`
- `expect_none`
- `unwrap_none`
- `unwrap_or_default`
- `as_const_deref`
- `as_mut_deref`
- `match`

## `Result<T>` methods

- `is_ok`
- `is_err`
- `contains`
- `contains_err`
- `ok`
- `err`
- `as_const_ref`
- `as_mut_ref`
- `map`
- `map_or`
- `map_or_else`
- `map_err`
- `AND`
- `and_then`
- `OR`
- `or_else`
- `unwrap_or`
- `unwrap_or_else`
- `unwrap`
- `expect`
- `unwrap_err`
- `expect_err`
- `unwrap_or_default`
- `as_const_deref`
- `as_const_deref_err`
- `as_mut_deref`
- `as_mut_deref_err`
- `match`

## FAQ

- libstdc++, Concepts

## Credits

- While the ideas aren't entirely mine, tartian llarma, p-ranav, ...
