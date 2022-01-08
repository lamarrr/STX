
#include <type_traits>

#define STX_ENABLE_IF(...) std::enable_if_t<__VA_ARGS__, int> = 0