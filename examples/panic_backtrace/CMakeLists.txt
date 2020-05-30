cmake_minimum_required(VERSION 3.1)

project(Example)

add_executable(example.bin main.cc)

execute_process(
  COMMAND git clone https://github.com/lamarrr/STX.git third_party/STX
  WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR})

set(STX_ENABLE_BACKTRACE
    ON
    CACHE BOOL "Enable the backtrace library" FORCE)
set(STX_ENABLE_PANIC_BACKTRACE
    ON
    CACHE BOOL "Enable panic backtrace" FORCE)

add_subdirectory(third_party/STX)

target_link_libraries(example.bin stx)

set_target_properties(example.bin
                      PROPERTIES CXX_STANDARD 20 CXX_STANDARD_REQUIRED ON)
