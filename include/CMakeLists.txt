add_project_library(stx allocator)
target_link_libraries(stx_allocator stx::resource stx::option_result stx::void
                      stx::enum stx::config)
