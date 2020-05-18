cmake_minimum_required(VERSION 3.1)

project(LLVMDemangle)

add_library(
  LLVMDemangle src/Demangle.cpp src/ItaniumDemangle.cpp
               src/MicrosoftDemangle.cpp src/MicrosoftDemangleNodes.cpp)

target_include_directories(LLVMDemangle PUBLIC include)
