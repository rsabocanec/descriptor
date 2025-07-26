cmake_minimum_required(VERSION 3.22)
project(udp-client)

set(CMAKE_CXX_STANDARD 23)

add_executable(udp-client main.cpp)

add_dependencies(udp-client descriptor)

target_link_libraries(udp-client PRIVATE descriptor)

target_include_directories(udp-client PRIVATE ${CMAKE_SOURCE_DIR})
