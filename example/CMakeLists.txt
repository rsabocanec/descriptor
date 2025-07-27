cmake_minimum_required(VERSION 3.22)

set(CMAKE_CXX_STANDARD 23)

add_subdirectory(tcp-client)
add_subdirectory(tcp-server)

add_subdirectory(local-stream-client)
add_subdirectory(local-stream-server)

add_subdirectory(udp-client)
add_subdirectory(udp-server)

add_subdirectory(local-datagram-client)
add_subdirectory(local-datagram-server)

add_subdirectory(can-publisher)
add_subdirectory(can_subscriber)
