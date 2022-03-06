add_project_library(stx vec)
target_link_libraries(stx_vec stx::allocator stx::void stx::struct stx::span)
