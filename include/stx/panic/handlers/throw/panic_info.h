/**
 * @file panic_info.h
 * @author Basit Ayantunde <rlamarrr@gmail.com>
 * @brief
 * @version  0.1
 * @date 2020-04-26
 *
 * @copyright Copyright (c) 2020
 *
 */
#ifndef STX_PANIC_HANDLERS_THROW_PANIC_INFO_H_
#define STX_PANIC_HANDLERS_THROW_PANIC_INFO_H_

#include <exception>
#include <string>

#include "stx/panic.h"

namespace stx {

class PanicInfo : public std::exception {
 public:
  // `message` must be a stack-allocated or long-lived string
  explicit PanicInfo(std::string_view info,
                     SourceLocation location = SourceLocation::current());

  PanicInfo(PanicInfo const&) = default;
  PanicInfo(PanicInfo&&) = default;
  PanicInfo& operator=(PanicInfo const&) = default;
  PanicInfo& operator=(PanicInfo&&) = default;
  ~PanicInfo() noexcept = default;

  const char* what() const noexcept override;

  std::string const& formatted_info() const noexcept;
  SourceLocation const& location() const noexcept;
  std::string_view const& info() const noexcept;
  size_t thread_id() const noexcept;

 private:
  std::string_view info_;
  size_t thread_id_;
  std::string formatted_info_;
  SourceLocation location_;
};

};  // namespace stx

#endif  // STX_PANIC_HANDLERS_THROW_PANIC_INFO_H_
