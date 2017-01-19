cmake_minimum_required(VERSION 2.6)
project(example)

add_executable(example_1 ./example_1.cpp)
target_link_libraries(example_1 zylog zyevent)
