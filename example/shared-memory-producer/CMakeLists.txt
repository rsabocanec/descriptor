cmake_minimum_required(VERSION 3.22)
project(shared-memory-producer)

set(CMAKE_CXX_STANDARD 23)

add_executable(shared-memory-producer main.cpp)

add_dependencies(shared-memory-producer descriptor)

target_link_libraries(shared-memory-producer PRIVATE descriptor)

target_include_directories(shared-memory-producer PRIVATE ${CMAKE_SOURCE_DIR})
