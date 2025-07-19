cmake_minimum_required(VERSION 3.28)
project(tcp-server)

set(CMAKE_CXX_STANDARD 23)

add_executable(tcp-server main.cpp)

add_dependencies(tcp-server descriptor)

target_include_directories(tcp-server PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(tcp-server PRIVATE descriptor)