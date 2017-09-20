cmake_minimum_required(VERSION 2.6)
project(zyevent)

add_library(zyevent ./zyevent.cpp)

add_subdirectory(test)
