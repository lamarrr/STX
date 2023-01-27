cmake_minimum_required(VERSION 3.1)

project(Example)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

execute_process(
  COMMAND git clone https://github.com/lamarrr/STX.git third_party/STX
  WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR})

add_subdirectory(third_party/STX)

add_executable(option option.cc)
target_link_libraries(option stx)

add_executable(panic_backtrace panic_backtrace.cc)
target_link_libraries(panic_backtrace stx)

add_executable(panic_hook panic_hook.cc)
target_link_libraries(panic_hook stx)

add_executable(panic panic.cc)
target_link_libraries(panic stx)

add_executable(result result.cc)
target_link_libraries(result stx)
