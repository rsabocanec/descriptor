cmake_minimum_required(VERSION 3.22)
project(can-publisher)

set(CMAKE_CXX_STANDARD 23)

add_executable(can-publisher main.cpp)

add_dependencies(can-publisher descriptor)

target_include_directories(can-publisher PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(can-publisher PRIVATE descriptor)
