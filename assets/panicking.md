@page Panicking

# Panicking

What is a panic?

Sometimes, bad things happen in your code, and there's nothing you can do about it. In these cases, STX has the `panic()` function.

When the `panic` function executes, your program will print a provided failure message, and then abort. This most commonly occurs when a bug of some kind has been detected and it's not clear to the programmer how to handle the error.

Panics serve for communicating an unexpected flow or corrupted state of the program. Consequentially, panics represent an end-point for the program. Common irrecoverable errors include: heap memory exhaustion, invalid memory indexes, password/access errors.

Let's try calling `panic()` in a simple program:

Filename: src/main.cc

```cpp
#include "stx/panic.h"

 int main() {
    stx::panic("crash and burn!");
 }
```

When you run the program, you'll see something like this:

```sh
$ ./main.bin

 thread with hash: '15729502191765196471' panicked with: 'crash and burn!' at function: 'main' [src/main.cc:5:1]
 Aborted (core dumped)
```

The call to `panic` causes the error message. The second line shows our panic message and the place in our source code where the panic occurred: `src/main.cc:5:1` indicates that it's the fifth line, first character of our src/main.cc file.

In this case, the line indicated is part of our code, and if we go to that line, we see the `panic` function call.

In other cases, the `panic` call might be in code that our code calls, and the filename and line number reported by the error message will be someone else's code where the `panic` function is called, not the line of our code that eventually led to the `panic` call. We can use the backtrace of the functions the `panic` call came from to figure out the part of our code that is causing the problem. Backtraces can be enabled via your application CMakeLists.txt file by setting `STX_ENABLE_BACKTRACE` to `ON` .

# When to panic() ?

Panics should be used when there is no means for the program to recover from an error, or for absolutely unexpected / **exceptional** cases. For recoverrable errors, see: `stx::Result` and `stx::Option`

When you choose to return a `Result` value, you give the calling code options rather than making the decision for it. The calling code could choose to attempt to recover in a way that's appropriate for its situation, or it could decide that an `Err` value in this case is unrecoverable, so it can call `panic` and turn your recoverable error into an unrecoverable one. Therefore, returning `Result` is a good default choice when you're defining a function that might fail.

# An Example

Here is an example of using a panic with a malloc.

```cpp
#include <cinttypes>
 #include <cstdlib>

 #include "stx/panic.h" 

 void * make_memory(size_t n) {
    auto memory = malloc(n);
    if(memory == nullptr) stx::panic("Memory Allocation Failure");
    return memory;
 }

 int main() {
     // panics if a memory allocation or heap exhaustion error occurs
     auto buffer = make_memory(1024);
     free(buffer);
 }
```

# Available Panic Handlers:

Given that systems range from user-facing to safety-critical (cannot crash) there's no one size fits all panicking behavior but there are plenty of commonly used behaviors.

To define your panic handlers, we have provided a few behaviours for your use:

- `stx::panic_default` : The default panic handler that prints the error message and panic location data, and afterwards aborts the program without freeing the resources. If the backtrace feature is enabled, the panic function will also print a stack backtrace. In addition, this panic handler is thread-safe when reporting to `stderr` .
- `stx::panic_halt` : A panic handler that causes the program, or the current thread, to halt by entering an infinite loop, Andrew Koenig generally advises halting on heap memory exhaustion.
- `stx::panic_abort` : A panic handler that causes the abort instruction to be executed.

# Overriding the default panic behaviour

Suppose you want to log the errors to disk or to a network before terminating the program, you can define your custom panic handlers to do that.

**NOTE**: A panic handler can only be defined once (One Definition Rule).

Firstly via your project's CMake build file, set the `STX_OVERRIDE_PANIC_HANDLER` option to `ON` , and then define the panic handler in your executable.

See the [`panic_handler`](https://github.com/lamarrr/STX/tree/master/examples) example project that illustrates how to do this.

We can also call any of the provided panic handlers.

# Debugging Notes

- For all calls to `stx::panic` within a program, the panic handler will be called to handle the panic. You can set your debugging breakpoint to the `stx::begin_panic` function to investigate all panics that occur within the program and also view a detailed backtrace.
- You can optionally enable automatic backtraces on panics. See the [`panic_backtrace`](https://github.com/lamarrr/STX/tree/master/examples) example project.

# Implementation and Portability Notes

- For printing/logging to `stderr` , the default panic handler only uses `fputs` and `fputc`.
- To use `panic` with the default panic handler on embedded systems you need to have `fputs` , `fputc` , and `snprintf` implemented. If you are using a POSIX-compliant operating system like nano OS in GNU newlib nano which already fills them with stubs, you have to implement the `write` syscall to forward to `UART` or whatever hardware peripheral you want `stderr` 's characters to be forwarded to.

- @see `stx::Result`

- @see `stx::Option`
