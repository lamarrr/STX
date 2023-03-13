#include <iostream>

#include "stx/option.h"

using stx::Option, stx::None, stx::Some;
using namespace std::literals;

auto unlock_bag(int password) -> Option<std::string_view>
{
  if (password == 123456)
  {
    return Some("Bag"sv);
  }
  else
  {
    return None;
  }
}

int main()
{
  std::cout << unlock_bag(123456).unwrap() << "\n";

  unlock_bag(000000).match(
      [](std::string_view bag) { std::cout << "unlocked bag:" << bag; },
      []() { std::cout << "unable to unlock"; });

  std::cout << std::endl;
}
