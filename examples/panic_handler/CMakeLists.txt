cmake_minimum_required(VERSION 3.1)

project(Example)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(example.bin main.cc)

execute_process(
  COMMAND git clone https://github.com/lamarrr/STX.git third_party/STX
  WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR})

set(STX_CUSTOM_PANIC_HANDLER
    ON
    CACHE BOOL "Override the default panic handler" FORCE)

add_subdirectory(third_party/STX)

target_link_libraries(example.bin stx)
