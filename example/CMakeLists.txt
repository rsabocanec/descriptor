cmake_minimum_required(VERSION 3.28)

set(CMAKE_CXX_STANDARD 23)

add_subdirectory(tcp-client)
add_subdirectory(tcp-server)

add_subdirectory(local-stream-client)
add_subdirectory(local-stream-server)