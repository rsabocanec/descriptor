cmake_minimum_required(VERSION 3.22)
project(can-subscriber)

set(CMAKE_CXX_STANDARD 23)

add_executable(can-subscriber main.cpp)

add_dependencies(can-subscriber descriptor)

target_include_directories(can-subscriber PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(can-subscriber PRIVATE descriptor)
