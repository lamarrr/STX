/**
 * @file panic_info.cc
 * @author Basit Ayantunde <rlamarrr@gmail.com>
 * @brief
 * @version  0.1
 * @date 2020-04-26
 *
 * @copyright Copyright (c) 2020
 *
 */

#include "stx/panic/handlers/throw/panic_info.h"

#include <thread>  // NOLINT

namespace stx {

using namespace std::string_literals;
using namespace std::string_view_literals;

PanicInfo::PanicInfo(std::string_view info, SourceLocation location)
    : std::exception{}, info_{info}, location_{location} {
  thread_id_ = std::hash<std::thread::id>()(std::this_thread::get_id());

  auto source_location = "' ["s + location_.file_name() + ":"s +
                         std::to_string(location_.line()) + ":" +
                         std::to_string(location_.column()) + "]";
  formatted_info_ = "thread: 'ID=" + std::to_string(thread_id_) +
                    "', panicked: '" + std::string(info_) + source_location;
}

const char* PanicInfo::what() const noexcept { return formatted_info_.c_str(); }

std::string const& PanicInfo::formatted_info() const noexcept {
  return formatted_info_;
}

SourceLocation const& PanicInfo::location() const noexcept { return location_; }

std::string_view const& PanicInfo::info() const noexcept { return info_; }

size_t PanicInfo::thread_id() const noexcept { return thread_id_; }

};  // namespace stx
