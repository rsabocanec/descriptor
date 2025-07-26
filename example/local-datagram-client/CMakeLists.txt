cmake_minimum_required(VERSION 3.22)
project(local-datagram-client)

set(CMAKE_CXX_STANDARD 23)

add_executable(local-datagram-client main.cpp)

add_dependencies(local-datagram-client descriptor)

target_link_libraries(local-datagram-client PRIVATE descriptor)

target_include_directories(local-datagram-client PRIVATE ${CMAKE_SOURCE_DIR})
