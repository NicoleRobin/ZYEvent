cmake_minimum_required (VERSION 2.6)
project(zyevent)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

include_directories(./include)
add_library(zyevent ./src/ZYEvent.cpp)
