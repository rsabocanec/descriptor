cmake_minimum_required(VERSION 3.28)
project(udp-server)

set(CMAKE_CXX_STANDARD 23)

add_executable(udp-server main.cpp)

add_dependencies(udp-server descriptor)

target_include_directories(udp-server PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(udp-server PRIVATE descriptor)