<img src="https://github.com/lamarrr/STX/workflows/tests_ubuntu/badge.svg">
<br/>
<div align="center"><img src="assets/stx.png"/> </div>

<div align="center"><i> C++ 20 error-handling and utility extensions to the C++ standard library.</i>
</div>

## Overview

STX...

<div align="center">
<h3><a href="http://lamarrr.github.io/STX">READ THE DOCUMENTATION</a></h3>
</div>

## Libraries

* Panicking
* `Result<T, E>` 
* `Option<T>` 
* Backtracing

## Features

* Efficient `Result<T, E>` (error-handling) and `Option<T>` (optional-value) implementation with monadic methods
* Fatal failure reporting via Panicking
* Runtime panic hooks
* Panic backtraces
* Signal backtraces i.e. `SIGSEGV` , `SIGILL` , `SIGFPE` 
* Backtrace library
* Suitable and easily adoptable for embedded systems
* Easy debugging
* Easy to use and hard to misuse API
* Exception-free, RTTI-free, and memory allocation free ( `no-std` )
* SFINAE-free
* Deterministic value lifetimes
* Eliminates repitive code and abstractable error-handling logic code via monadic extensions
* Fast success and error return paths
* Modern and clean API

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
#include <iostream>

#include "stx/result.h"

using std::array, std::string_view;
using stx::Result, stx::Ok, stx::Err;
using namespace std::literals;

enum class Version { Version1 = 1, Version2 = 2 };

auto parse_version(array<uint8_t, 6> const& header) -> Result<Version, string_view> {
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

## Guidelines

* Some methods like `match` , `map` , `unwrap` and most of the `Result` , and `Option` methods **consume** the stored value and thus the `Result` or `Option` has to be destroyed as its lifetime has ended. For example:

  Say we define a function named `safe_divide` as in the example above, with the following prototype:

``` cpp
auto safe_divide(float n, float d) -> Option<float>;
```

And we call:

``` cpp
float result = safe_divide(n, d).unwrap(); // compiles, because safe_divide returns a temporary
```

``` cpp
Option option = safe_divide(n, d);
float result = option.unwrap();  // will not compile, because unwrap consumes the value and is only usable with temporaries (as above) or r-value references (as below)
```

Alternatively, suppose the `Option` or `Result` is no longer needed, we can obtain an r-value reference:

``` cpp

Option option = safe_divide(n, d);
float result  = std::move(option).unwrap(); // will compile, the value is moved out of `option` , `option` should not be used any more

```

<span style="color:red">**NOTE**</span>: Just as any moved-from object, `Option` and `Result` are not to be used after a `std::move` ! (as the objects will be left in an unspecified state).

* `Option` and `Result` do not perform any implicit copies of the contained object as they are designed as purely forwarding types, this is especially due to their primary purpose as return channels in which we do not want duplication or implicit copies of the returned values. 

To make explicit copies:

``` cpp

Option option = safe_divide(n, d);
float result = option.clone().unwrap(); // note that `clone()` explicitly makes a copy of the `Option` 

```

We can also obtain an l-value reference to copy the value:

``` cpp

Option option = safe_divide(n, d);
float result = option.value(); // note that `value()` returns an l-value reference and `result` is copied from `option` 's value in the process

```

``` cpp

float result = safe_divide(n, d).value(); // this won't compile as `value` always returns an l-value reference, use `unwrap()` instead

```

## Benchmarks

### Release Mode ( `-O3` )

| Target | Real Time (ns) | CPU Time (ns) | Iterations |
|--------|----------------|---------------|------------|
Variant/SuccessPath   |     0.392 ns  |      0.392 ns |  1000000000
Exception/SuccessPath |     0.386 ns  |      0.386 ns |  1000000000
Result/SuccessPath    |     0.381 ns  |      0.381 ns |  1000000000
C-Style/SuccessPath   |     0.387 ns  |      0.386 ns |  1000000000
Variant/FailurePath   |     0.408 ns  |      0.408 ns |  1000000000
Exception/FailurePath |      2129 ns  |       2129 ns |      317810
Result/FailurePath    |     0.384 ns  |      0.384 ns |  1000000000
C-Style/FailurePath   |     0.384 ns  |      0.383 ns |  1000000000

## Build Requirements

* CMake
* Make or Ninja Build
* C++ 20 Compiler
* Doxygen and Graphviz (only for documentation)

## Tested-on Compiler & Toolchains

* Clang-10 + libstdc++-9
* Clang-11 + libstdc++-9

## CMake Configuration Options

* `STX_BUILD_SHARED` - Build STX as a shared library
* `STX_BUILD_TESTS` - Build test suite
* `STX_BUILD_DOCS` - Build documentation
* `STX_BUILD_BENCHMARKS` - Build benchmarks
* `STX_SANITIZE_TESTS` - Sanitize tests if supported. Builds address-sanitized, thread-sanitized, leak-sanitized, and undefined-sanitized tests
* `STX_OVERRIDE_PANIC_HANDLER` - Override the global panic handler
* `STX_ENABLE_BACKTRACE` - Disable the backtrace backend
* `STX_ENABLE_PANIC_BACKTRACE` - Disables panic backtraces. It depends on the backtrace backend.

## FAQs

## License

[**MIT License**](LICENSE)
