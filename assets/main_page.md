@mainpage

# What is STX?

STX is a collection of libraries and utilities designed to make working with C++ easier and less error-prone. This includes but is not limited to well-proven and widely adopted paradigms, data-structures, and designs from other prominent projects and programming languages accross the software engineering community.

At the core, all STX libraries are `no_std` . No RTTI, memory allocation, nor exceptions.

# STX Libraries

## Panic Library

See guide on @ref Panicking

The panic library provides:

* panicking:

  + `stx::panic` : signals an abnormal flow/error in the program.

* runtime panic hooks
  + `stx::take_panic_hook` , `stx::attach_panic_hook` , and `stx::panic_hook_visible` 

* panic backtraces (see the [ `panic_backtrace` ](https://github.com/lamarrr/STX/tree/master/examples) example project)

* error reporting (see `stx::FixedReport` and `stx::SpanReport`)

* panic handlers:

  + `stx::panic_default` : The default panic handler that prints the error message and panic location data, and afterwards aborts the program.
  + `stx::panic_halt` : A panic handler that causes the current calling thread, to halt by entering an infinite loop.
  + `stx::panic_abort` : A panic handler that causes the abort instruction to be executed.

## Error and Optional-Value Handling Library

These monadic types not only make error handling easier but also make the paths more obvious to the compiler for optimizations.
Monads can be simply thought of as abstract types of actions. Their monadic nature makes it easy to operate on them as pipelines and in the process eliminate redundant error-handling logic code.

* `stx::Result<T, E>` : Type for relaying the result of a function that can fail or succeed (with monadic extensions)
* `stx::Option<T>` : Type for **safe** optional values (with monadic extensions)

## Backtracing Library

The backtracing library is useful for manually querying/viewing information of active stack frames. It makes debugging easier by making it easier to get stackframe information programmartically or automatically (via panics) without having to inspect core dumps or step into a debugger in which errors might not be reproducible (especially for embedded systems). The backtrace library is disabled by default as not all platforms support them, It can be enabled by setting `STX_ENABLE_BACKTRACE` to `ON` in the CMakeLists.txt file, this is demonstrated in the [ `panic_backtrace` ](https://github.com/lamarrr/STX/tree/master/examples) example project.

* Fatal signal backtraces for `SIGSEGV` , `SIGFPE` , and `SIGILL` (`stx::backtrace::handle_signal`)
* Manual stack backtraces (`stx::backtrace::trace`)

# Why STX?

STX is primarilly an effort to bring a more convenient and usable error-handling model to the C++ ecosystem. Whilst also sampling ideas from various error-handling model implementations duplicated across the community, Most of the basic facilities for which STX's libraries aims to provide.

Here is a list of C++ projects with a similar error-handling model:

* [XPCC's Error Model](https://blog.salkinium.com/xpccs-error-model/) (Abandonment)
* Boost's [ `expected` ](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2014/n4109.pdf) and [ `outcome` ](https://www.boost.org/doc/libs/1_70_0/libs/outcome/doc/html/index.html)
* Simon Brand's [ `tl::optional` ](https://github.com/TartanLlama/optional)
* Abseil's [ `Status` , `StatusCode` ](https://github.com/abseil/abseil-cpp/tree/master/absl/status), and [ `RawLog` ](https://github.com/abseil/abseil-cpp/blob/master/absl/base/internal/raw_logging.cc)
* Tensorflow Lite Micro's [ `ErrorReporter` ](https://github.com/tensorflow/tensorflow/blob/master/tensorflow/lite/core/api/error_reporter.h)
* Tensorflow's [ `Status` , `Code` ](https://github.com/tensorflow/tensorflow/blob/master/tensorflow/core/platform/status.h), [ `Logging` ](https://github.com/tensorflow/tensorflow/blob/master/tensorflow/core/platform/default/logging.cc), [ `ENSURE_OK` , `CHECK_OK` , `DCHECK_OK` , `QCHECK_OK` ](https://github.com/tensorflow/tensorflow/blob/master/tensorflow/core/platform/status.h), and other error handling macros
* Mozilla's [ `Result<T, E>` ](https://searchfox.org/mozilla-central/source/mfbt/Result.h), [ `Maybe<T>` ](https://searchfox.org/mozilla-central/source/mfbt/Maybe.h), [ `MaybeOneOf<A, B>` ](https://searchfox.org/mozilla-central/source/mfbt/MaybeOneOf.h), and [ `NotNull<T*>` ](https://searchfox.org/mozilla-central/source/mfbt/NotNull.h)
* Google Pigweed's [ `Result<T, Status>` ](https://pigweed.googlesource.com/pigweed/pigweed/+/refs/heads/master/pw_result/), [ `Status` ](https://pigweed.googlesource.com/pigweed/pigweed/+/refs/heads/master/pw_status/)
* Simdjson's [ `error_code` , `result` ](https://github.com/simdjson/simdjson/blob/master/include/simdjson/error.h)
* MBED OS' [ `Error` ](https://github.com/ARMmbed/mbed-os/blob/master/platform/source/mbed_error.c)
* LLVM's [ `Error` , `Expected` ](https://github.com/llvm/llvm-project/blob/master/llvm/include/llvm/Support/Error.h), [ `ErrorOr` ](https://github.com/llvm/llvm-project/blob/master/llvm/include/llvm/Support/ErrorOr.h), [ `ErrorHandling` ](https://github.com/llvm/llvm-project/blob/master/llvm/lib/Support/ErrorHandling.cpp)
* Fuchsia OS - Zircon Common Library' s [ `ZX_PANIC` ](https://fuchsia.googlesource.com/fuchsia/+/HEAD/zircon/system/public/zircon/assert.h), [ `Result` ](https://fuchsia.googlesource.com/fuchsia/+/refs/heads/master/zircon/system/ulib/zxc/), [ `OpenResult` ](https://fuchsia.googlesource.com/fuchsia/+/refs/heads/master/zircon/system/ulib/fs/include/fs/vfs.h), [ `StatusOr` ](https://fuchsia.googlesource.com/fuchsia/+/refs/heads/master/zircon/system/ulib/intel-hda/include/intel-hda/utils/status_or.h), and [ `Status` ](https://fuchsia.googlesource.com/fuchsia/+/refs/heads/master/zircon/system/ulib/zxc/)
* P-ranav's [ `Result<T, E>` ](https://github.com/p-ranav/result/blob/master/include/result/result.hpp)
* Mum4k's Arduino Error Classes: [ `ErrorOr` ](https://github.com/mum4k/arduino_error/blob/master/error_or.h), [ `Error` ](https://github.com/mum4k/arduino_error/blob/master/error.h), and [Error Macros](https://github.com/mum4k/arduino_error/blob/master/error_macros.h)


# FAQs

* Why not exceptions?

These reasons are a bit biased, but based on my team's experience:

  + Exceptions are really great, but present implementations are not so great for fail-often functions
  + Some embedded systems toolchains don't have a conformant exception implementation and throwing an exception resorts to an immediate abort
  + Currently, Exceptions are not deterministic in both space and time
  + You never know which function throws which exception and you never know when the callee code's API changes the exception type which can have extremely serious effects that'll go unnoticed
  + Exceptions are implicitly and automatically propagated up the call stack until it finds a function to handle it which you might not want as **error-handling is very very contextual**
  + Exception-handling code gets messy easily
  + Exceptions are essentially for exceptional cases
  + Exceptions require RTTI
  + Some custom exception implementations make use of dynamic memory allocation (many using std::string especially)
  + There's nothing stopping you from catching exceptions you are not meant to catch

# Some Interesting Reads

* Joe Duffy: [The Error Model](http://joeduffyblog.com/2016/02/07/the-error-model/)

## Relatable Excerpts

> This problem isn’t theoretical. I’ve encountered numerous bugs caused by ignoring return codes and I’m sure you have too. Indeed, in the development of this very Error Model, my team encountered some fascinating ones. For example, when we ported Microsoft’s Speech Server to Midori, we found that 80% of Taiwan Chinese (zh-tw) requests were failing. Not failing in a way the developers immediately saw, however; instead, clients would get a gibberish response. At first, we thought it was our fault. But then we discovered a silently swallowed HRESULT in the original code. Once we got it over to Midori, the bug was thrown into our faces, found, and fixed immediately after porting. This experience certainly informed our opinion about error codes.

> In the exception model, any function call – and sometimes any statement – can throw an exception, transferring control non-locally somewhere else. Where? Who knows. There are no annotations or type system artifacts to guide your analysis. As a result, it’s difficult for anyone to reason about a program’s state at the time of the throw, the state changes that occur while that exception is propagated up the call stack – and possibly across threads in a concurrent program – and the resulting state by the time it gets caught or goes unhandled.

> Bugs Aren’t Recoverable Errors!


* Herb Sutter: [Zero Overhead Deterministic Exceptions](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2018/p0709r0.pdf)

# Relatable Excerpts

> Many projects ban exceptions. In [SC++F 2018](https://isocpp.org/blog/2018/03/results-summary-cpp-foundation-developer-survey-lite-2018-02), 52% of C++ developers reported that exceptions were banned in part or all of their project code i.e., most are not allowed to freely use C++’s primary recommended error-handling model that is required to use the C++ standard language and library.

> A programming bug or abstract machine corruption is never an "error" (both are not programmatically recoverable, so report bugs to the human programmer using **contract** violations that default to **fail-fast** and report abstract machine corruption using fail-fast). Programming bugs (e.g., out-of-bounds access, null dereference) and abstract machine corruption cause a corrupted state that cannot be recovered from programmatically, and so **should never** be reported to the calling code as errors that code could somehow handle.

Note here that exceptions also don't fit this fail-fast procedure as they can be caught by any function on the stack! and try to continue the program!!! (for fail-fast: STX provides `stx::panic`, and for recoverable errors: `stx::Result`). There should be no reason to continue the program or propagate a programming bug.

> Today's dynamic exception handling is not deterministic (run-time space and time determinism). This is the primary reason exceptions are banned in many real-time and/or safety-critical environments (for example, many games, coding standards like JSF++ [[JSF++2005]](http://www.stroustrup.com/JSF-AV-rules.pdf), and environments like the Mars Rover flight software [[Maimone 2014]](https://github.com/CppCon/CppCon2014/blob/master/Presentations/C%2B%2B%20on%20Mars%20-%20Incorporating%20C%2B%2B%20into%20Mars%20Rover%20Flight%20Software/C%2B%2B%20On%20Mars%20-%20Mark%20Maimone%20-%20CppCon%202014.pdf)).

> We cannot accept that "Standard C++ is not applicable for real-time systems" -- that would be an admission of defeat in C++'s core mission as an efficient systems programming language. Therefore we know we cannot live with today's model unchanged.

> "[In CLU] We rejected this approach [propagating dynamic exceptions] because it did not fit our ideas about modular program construction. We wanted to be able to call a procedure knowing just its specification, not its implementation." -- [[Liskov 1992]](http://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.127.8460&rep=rep1&type=pdf)

> "By allowing multi-level propagation of exceptions, C++ loses one aspect of static checking. One cannot simply look at a function to determine which exceptions it may throw." -- [Stroustrup 1994] p. 395

