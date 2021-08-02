add_project_library(stx panic)
target_link_libraries(stx_panic stx::utils stx::source_location)