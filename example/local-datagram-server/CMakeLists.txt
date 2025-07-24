cmake_minimum_required(VERSION 3.28)
project(local-datagram-server)

set(CMAKE_CXX_STANDARD 23)

add_executable(local-datagram-server main.cpp)

add_dependencies(local-datagram-server descriptor)

target_include_directories(local-datagram-server PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(local-datagram-server PRIVATE descriptor)