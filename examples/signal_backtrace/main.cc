
#include <csignal>

#include "stx/backtrace.h"

using stx::backtrace::handle_signal;

int main() {
  auto old = handle_signal(SIGSEGV).unwrap();
  old = handle_signal(SIGILL).unwrap();
  old = handle_signal(SIGFPE).unwrap();

  (void)old;

  int *ptr = nullptr;

  *ptr = 10;
}