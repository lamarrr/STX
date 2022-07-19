add_project_library(stx async)
target_link_libraries(
  stx_async
  stx::struct
  stx::mem
  stx::enum
  stx::limits
  stx::spinlock
  stx::config)
