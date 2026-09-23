cmake_minimum_required(VERSION 3.22)
project(fifo)

set(CMAKE_CXX_STANDARD 23)

add_executable(fifo main.cpp)

add_dependencies(fifo descriptor)

target_include_directories(fifo PRIVATE ${CMAKE_SOURCE_DIR})

target_link_libraries(fifo PRIVATE descriptor CLI11::CLI11 fmt::fmt spdlog::spdlog)
