#include <iostream>

#include "stx/panic.h"
#include "stx/panic/hook.h"

void hook(std::string_view info, std::string_view error_report,
          stx::SourceLocation location)
{
  std::cout << "========= panic from custom hook =========\n";
  std::cout << info << '\n';
  std::cout << error_report << '\n';
}

int main()
{
  if (!stx::attach_panic_hook(&hook))
  {
    std::cout << "unable to set panic hook" << std::endl;
  }
  stx::panic("crash and burn!");
}