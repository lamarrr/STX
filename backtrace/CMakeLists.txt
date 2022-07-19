add_project_library(stx backtrace)
target_link_libraries(
  stx_backtrace
  absl::stacktrace
  absl::symbolize
  stx::option_result
  stx::panic
  stx::span
  stx::fn
  stx::config)
