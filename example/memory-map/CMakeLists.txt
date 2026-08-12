cmake_minimum_required(VERSION 3.22)
project(memory-map)

set(CMAKE_CXX_STANDARD 23)

add_executable(memory-map main.cpp)

add_dependencies(memory-map descriptor)

target_link_libraries(memory-map PRIVATE descriptor)

target_include_directories(memory-map PRIVATE ${CMAKE_SOURCE_DIR})
