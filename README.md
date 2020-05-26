<div align="center"><img src="assets/stx.png"/> </div>

C++ 20 Error Handling and Monadic extensions to the C++ standard library

<div align="center"> <h3>
<a href="http://lamarrr.github.io/blog/error-handling">READ THE DOCUMENTATION</a></h3> 
</div>
## Overview



## Features

- Efficient `Result<T, E>` (error-handling) and `Option<T>` (optional-value) implementation
- Panicking via `panic`
- Runtime panic hooks
- Panic backtraces
- Signal backtraces i.e. `SIGSEGV`, `SIGILL`, `SIGFPE`
- Backtrace library
- Suitable and easily adoptable for embedded systems
- Easy debugging
- Easy to use, Hard-to-misuse
- Exception-free, RTTI-free, and malloc-free (`no-std`)

## Guidelines

- Some methods like `match`, `map`, `unwrap` and most of the `Result`, and `Option` methods **consume** the stored value and thus the `Result` or `Option` has to be destroyed as its lifetime has ended. For example:

	Say we define a function named `safe_divide` with the following prototype:

```cpp
auto safe_divide(float n, float d) -> Option<float>;
```

And we call:



```cpp
float result = safe_divide(n, d).unwrap(); // compiles, because safe_divide returns a temporary
```

```cpp
Option option = safe_divide(n, d); 
float result = option.unwrap();  // will not compile, because unwrap consumes the value and is only usable with temporaries (as above) or r-value references (as below)
```

Alternatively, suppose the `Option` or `Result` is no longer needed, we can obtain an r-value reference:

```cpp

Option option = safe_divide(n, d);
float result  = std::move(option).unwrap(); // will compile, the value is moved out of `option`, `option` should not be used any more

```


<span style="color:red">**NOTE**</span>: Just as any moved-from object, `Option` and `Result` are not to be used after a std::move! (as the objects will be left in an unspecified state).


- `Option` and `Result` do not perform any implicit copies of the contained object as they are designed as purely forwarding types, this is especially due to their primary purpose as return channels in which we do not want duplication or implicit copies of the returned values. 

To make explicit copies:

```cpp

Option option = safe_divide(n, d);
float result = option.clone().unwrap(); // note that `clone()` explicitly makes a copy of the `Option`

```


We can also obtain an l-value reference to copy the value:

```cpp

Option option = safe_divide(n, d);
float result = option.value(); // note that `value()` returns an l-value reference and `result` is copied from `option`'s value in the process

```

```cpp

float result = safe_divide(n, d).value(); // this won't compile as `value` always returns an l-value reference, use `unwrap()` instead


```



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

## Build Requirements

- CMake
- Doxygen
- Make or Ninja Build
- Clang-10
- Libstdc++-9 (GCC 9)

## Configuration Options

- Must specify C++20!!! source loc and concepts


## FAQ

- libstdc++, Concepts

## Credits

- While the ideas aren't entirely mine, tartian llarma, p-ranav, ...
