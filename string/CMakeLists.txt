add_project_library(stx "string")
target_link_libraries(stx_string stx::allocator stx::mem stx::allocator
                      stx::span stx::config)
