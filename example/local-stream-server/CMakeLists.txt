cmake_minimum_required(VERSION 3.22)
project(local-stream-server)

set(CMAKE_CXX_STANDARD 23)

add_executable(local-stream-server main.cpp)

add_dependencies(local-stream-server descriptor)

target_include_directories(local-stream-server PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(local-stream-server PRIVATE descriptor)
