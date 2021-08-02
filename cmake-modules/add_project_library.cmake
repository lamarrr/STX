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
  set_target_properties(
    ${library_name} PROPERTIES CXX_STANDARD
                               ${${target_project_name}_CXX_STANDARD})

  target_include_directories(${library_name}
                             PUBLIC ${${library_name}_PUBLIC_INCLUDE_DIRECTORY})
  target_include_directories(
    ${library_name} PRIVATE ${${library_name}_PRIVATE_INCLUDE_DIRECTORY})

  string(TOUPPER ${target_project_name} target_project_name_upper)

  file(GLOB ${library_name}_TESTS_SOURCE_FILES_LIST tests/*.cc)

  if(${${target_project_name_upper}_BUILD_TESTS})
    if(${library_name}_TESTS_SOURCE_FILES_LIST)
      add_executable(${library_name}_tests
                     ${${library_name}_TESTS_SOURCE_FILES_LIST})
      target_include_directories(${library_name}_tests PRIVATE tests)
      target_link_libraries(${library_name}_tests gtest_main gtest
                            ${library_alias})
    endif()
  endif()

endmacro()
