cmake_minimum_required(VERSION 3.28)
project(tcp-client)

set(CMAKE_CXX_STANDARD 23)

add_executable(tcp-client main.cpp)

add_dependencies(tcp-client descriptor)

target_link_libraries(tcp-client PRIVATE descriptor)

target_include_directories(tcp-client PRIVATE ${CMAKE_SOURCE_DIR})