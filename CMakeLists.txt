cmake_minimum_required(VERSION 3.28)
project(descriptor)

set(CMAKE_CXX_STANDARD 23)

add_library(descriptor SHARED descriptor.cpp
        socket.cpp
        stream_socket.cpp
        tcp_socket.cpp
        bound_socket.cpp
        datagram_socket.cpp
        local_stream_socket.cpp)

add_library(descriptor-static STATIC descriptor.cpp
        socket.cpp
        stream_socket.cpp
        tcp_socket.cpp
        bound_socket.cpp
        datagram_socket.cpp
        local_stream_socket.cpp)

set_target_properties(descriptor-static PROPERTIES OUTPUT_NAME descriptor)

add_subdirectory(example)
