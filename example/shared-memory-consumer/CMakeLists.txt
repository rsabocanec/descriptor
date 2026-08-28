cmake_minimum_required(VERSION 3.22)
project(shared-memory-consumer)

set(CMAKE_CXX_STANDARD 23)

add_executable(shared-memory-consumer main.cpp)

add_dependencies(shared-memory-consumer descriptor)

target_link_libraries(shared-memory-consumer PRIVATE descriptor)

target_include_directories(shared-memory-consumer PRIVATE ${CMAKE_SOURCE_DIR})
