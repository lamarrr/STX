macro(add_project_library target_project_name target_library_name)
  set(library_name ${target_project_name}_${target_library_name})
  set(library_alias ${target_project_name}::${target_library_name})
  file(GLOB ${library_name}_SOURCE_FILES_LIST src/*.cc)
  set(${library_name}_PUBLIC_INCLUDE_DIRECTORY "include")
  set(${library_name}_PRIVATE_INCLUDE_DIRECTORY "src")
  set(${library_name}_SOURCE_FILES ${${library_name}_SOURCE_FILES_LIST})

  message(VERBOSE "${library_name} public include directory: "
          ${${library_name}_PUBLIC_INCLUDE_DIRECTORY})
  message(VERBOSE "${library_name} private include directory: "
          ${${library_name}_PRIVATE_INCLUDE_DIRECTORY})
  message(VERBOSE "${library_name} source files: "
          ${${library_name}_SOURCE_FILES})

  add_library(${library_name} STATIC ${${library_name}_SOURCE_FILES})
  add_library(${library_alias} ALIAS ${library_name})

  target_include_directories(${library_name}
                             PUBLIC ${${library_name}_PUBLIC_INCLUDE_DIRECTORY})
  target_include_directories(
    ${library_name} PRIVATE ${${library_name}_PRIVATE_INCLUDE_DIRECTORY})
endmacro()
