add_project_library(stx scheduler)
target_link_libraries(
  stx_scheduler
  stx::fn
  stx::mem
  stx::option_result
  stx::span
  stx::task
  stx::vec
  "stx::string"
  stx::void)
