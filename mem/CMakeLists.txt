add_project_library(stx mem)
target_link_libraries(stx_mem stx::resource stx::allocator stx::struct
                      stx::config)
