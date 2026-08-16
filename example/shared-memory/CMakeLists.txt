cmake_minimum_required(VERSION 3.22)
project(shared-memory)

set(CMAKE_CXX_STANDARD 23)

add_executable(shared-memory main.cpp)

add_dependencies(shared-memory descriptor)

target_link_libraries(shared-memory PRIVATE descriptor)

target_include_directories(shared-memory PRIVATE ${CMAKE_SOURCE_DIR})
