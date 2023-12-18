<img src="https://github.com/lamarrr/STX/workflows/cpp-17:clang-14:macos-12/badge.svg"> <img src="https://github.com/lamarrr/STX/workflows/cpp-17:clang-14:ubuntu-22.04/badge.svg"> <img src="https://github.com/lamarrr/STX/workflows/cpp-17:gcc-12:ubuntu-22.04/badge.svg"> <img src="https://github.com/lamarrr/STX/workflows/cpp-17:msvc-2019:windows/badge.svg">
<img src="https://github.com/lamarrr/STX/workflows/cpp-17:clang-15:ubuntu-22.04-qemu-arm/badge.svg">

<br/>
<div align="center"><img src="assets/stx.png"/> </div>

<div align="center"><i> C++ 17 & C++ 20 error-handling and utility extensions.</i>
</div>

## Overview

STX is a collection of libraries and utilities designed to make working with C++ easier and less error-prone.

<div align="center">
<h3>
<a href="http://lamarrr.github.io/STX"> READ THE DOCUMENTATION </a>
</h3>
</div>

<div align="center">
<h3>
<a href="https://github.com/lamarrr/STX/tree/master/examples"> SEE THE EXAMPLES </a>
</h3>
</div>

## Features

* Experimental async library, runtime, and scheduler
* C-API-compatible functor object `Fn<Return(Args...)>`
* Memory allocators
* Resource reference-counting `Rc<Handle>`
* `Span<T>` with helper methods that eliminates use of iterator-pairs
* `Vec<T>` with trivial relocation (faster than `std::vector<T>`)
* Efficient `Result<T, Err>` (error-handling) and with monadic methods
* Efficient `Option<T>` (optional-value handling) with monadic methods
* UTF-8 string iterator method
* Fail-fast (Abandonment/ Fatal failure) via `panic` s
* Runtime panic hooks
* Backtrace library
* Source Location
* Panic with backtraces
* Portable, suitable, and easily-adoptable for embedded systems, real-time systems, safety-critical systems, and operating systems
* Easy debugging
* Easy to use and hard to misuse API
* Exception-free, RTTI-free, and memory allocation free ( `no-std` )
* Space and time deterministic error-handling
* Deterministic value lifetimes
* Eliminates repetitive code and abstractable error-handling logic code via monadic extensions
* Fast success and error return paths
* Modern and clean API
* Well-documented
* Extensively tested

## Basic Examples

### Option

``` cpp

#include <iostream>

#include "stx/option.h"

using stx::Option, stx::Some, stx::None;

auto safe_divide(double numerator, double denominator) -> Option<double> {
  if (denominator == 0.0) return None;
  return Some(numerator / denominator);
}

int main() {
  safe_divide(5.0, 2.0).match(
      [](auto value) { std::cout << "Result: " << value << std::endl; },
      []() { std::cout << "Cannot divide by zero" << std::endl; });
}

```

### Result

``` cpp

#include <array>
#include <cinttypes>
#include <iostream>

#include "stx/result.h"

using std::array, std::string_view;
using namespace std::literals;

using stx::Result, stx::Ok, stx::Err;

enum class Version { V1 = 1, V2 = 2 };

auto parse_version(array<uint8_t, 6> const& header) -> Result<Version, string_view> {
  switch (header.at(0)) {
    case 1:
      return Ok(Version::V1);
    case 2:
      return Ok(Version::V2);
    default:
      return Err("Unknown Version"sv);
  }
}

int main() {
  parse_version({2, 3, 4, 5, 6, 7}).match([](auto version){
    std::cout << "Version: " << static_cast<int>(version) << std::endl;
  }, [](auto error){
    std::cout << error  << std::endl;
  });

}

```

### Propagating Errors with `TRY_OK`

`TRY_OK` assigns the successful value to its first parameter `version` if `parse_version` returned an `Ok` , else propagates the error value.

``` cpp

// As in the example above
auto parse_version(array<uint8_t, 6> const& header) -> Result<Version, string_view>;

auto parse_data(array<uint8_t, 6> const& header) -> Result<uint8_t, string_view> {
  TRY_OK(version, parse_version(header));
  return Ok(version + header[1] + header[2]);
}

int main() {
  auto parsed = parse_data({2, 3, 4, 5, 6, 7}).unwrap();

  std::cout << parsed << std::endl;
}

```

You can also add const/volatile attributes to `TRY_OK` 's assigned value, i.e:

``` cpp

auto parse_data(array<uint8_t, 6> const& header) -> Result<uint8_t, string_view> {
  TRY_OK(const version, parse_version(header));
  return Ok(version + header[1] + header[2]);
}

```

## Guidelines

* Result and Option will only work in `constexpr` context (compile-time error-handling) in C++ 20, to check if you can use it as `constexpr` check if the macros `STX_RESULT_IS_CONSTEXPR` and `STX_OPTION_IS_CONSTEXPR` are set to `1`, for an example see [`constexpr_test`](tests/constexpr_test.cc) .
* To ensure you never forget to use the returned errors/results, raise the warning levels for your project ( `-Wall`  `-Wextra`  `-Wpedantic` on GNUC-based compilers, and `/W4` on MSVC)
* Some methods like `map` , `unwrap`, `or_else`, and most of `Result` and `Option`'s monadic methods **consume** the stored value and thus the `Result` or `Option` has to be destroyed as its lifetime has ended. For example:

  Say we define a function named `safe_divide` as in the example above, with the following prototype:

``` cpp
auto safe_divide(float n, float d) -> Option<float>;
```

And we call:

``` cpp
float result = safe_divide(n, d).unwrap(); // compiles, because 'safe_divide' returns a temporary
```

``` cpp
Option option = safe_divide(n, d);
float result = option.unwrap();  // will not compile, because 'unwrap' consumes the value and is only usable with temporaries (as above) or r-value references (as below)
```

Alternatively, suppose the `Option` or `Result` is no longer needed, we can obtain an r-value reference:

``` cpp

Option option = safe_divide(n, d);
float result  = std::move(option).unwrap(); // will compile, the value is moved out of 'option' , 'option' should not be used any more

```

<b>NOTE</b>: Just as any moved-from object, `Option` and `Result` are not to be used after a `std::move` ! (as the objects will be left in an unspecified state).

* `Result` and `Option` do not make any implicit copies of the contained object as they are designed as purely forwarding types, this is especially due to their primary purpose as return channels in which we do not want duplication nor implicit copies of the returned values.

To make explicit copies:

``` cpp

Option option = safe_divide(n, d);
float result = option.clone().unwrap(); // note that 'clone()' explicitly makes a copy of the 'Option'

```

We can also obtain an l-value reference to copy the value:

``` cpp

Option option = safe_divide(n, d);
float result = option.value(); // note that 'value()' returns an l-value reference and 'result' is copied from 'option''s value in the process

```

``` cpp

float result = safe_divide(n, d).value(); // this won't compile as 'value' always returns an l-value reference, use 'unwrap()' instead

```

* All methods of `Result` and `Option` pass r-value/l-value references to their invocable parameters.

## Build Requirements

* CMake
* Make or Ninja Build
* C++ 17 or C++ 20 Compiler
* Git
* Doxygen and Graphviz (for documentation)

## License

[**MIT License**](LICENSE)

## FAQ

### Is STX's ABI stable?

NO
