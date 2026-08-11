cmake_minimum_required(VERSION 3.22)
project(tcp-splice)

set(CMAKE_CXX_STANDARD 23)

add_executable(tcp-splice main.cpp)

add_dependencies(tcp-splice descriptor)

target_link_libraries(tcp-splice PRIVATE descriptor)

target_include_directories(tcp-splice PRIVATE ${CMAKE_SOURCE_DIR})
