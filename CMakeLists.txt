cmake_minimum_required(VERSION 3.22)
project(descriptor)

set(CMAKE_CXX_STANDARD 23)

set(SUPPORT_STACKTRACE False)

add_library(descriptor SHARED descriptor.cpp
        socket.cpp
        stream_socket.cpp
        tcp_socket.cpp
        bound_socket.cpp
        datagram_socket.cpp
        local_stream_socket.cpp
        udp_socket.cpp
        local_datagram_socket.cpp
        local_datagram_socket.h
        can_socket.cpp
        file.cpp
        shared_memory.cpp
        timer.cpp
        semaphores.cpp)

add_library(descriptor-static STATIC descriptor.cpp
        socket.cpp
        stream_socket.cpp
        tcp_socket.cpp
        bound_socket.cpp
        datagram_socket.cpp
        local_stream_socket.cpp
        udp_socket.cpp
        local_datagram_socket.cpp
        local_datagram_socket.h
        can_socket.cpp
        file.cpp
        shared_memory.cpp
        timer.cpp
        semaphores.cpp)

if(SUPPORT_STACKTRACE)
    target_compile_definitions(descriptor PUBLIC _STACKTRACE_SUPPORTED)
    target_link_libraries(descriptor INTERFACE stdc++_libbacktrace)

    target_compile_definitions(descriptor-static PUBLIC _STACKTRACE_SUPPORTED)
    target_link_libraries(descriptor-static INTERFACE stdc++_libbacktrace)
endif()

set_target_properties(descriptor-static PROPERTIES OUTPUT_NAME descriptor)

add_subdirectory(example)
