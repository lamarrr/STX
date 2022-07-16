add_project_library(stx panic)
target_link_libraries(stx_panic stx::utils stx::source_location)

if(STX_CUSTOM_PANIC_HANDLER)
  target_compile_definitions(stx_panic PRIVATE STX_CUSTOM_PANIC_HANDLER)
endif()
