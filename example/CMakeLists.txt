cmake_minimum_required(VERSION 3.22)

set(CMAKE_CXX_STANDARD 23)

include(${CMAKE_SOURCE_DIR}/cmake/CPM.cmake)

if(NOT TARGET fmtlib::fmtlib)
cpmaddpackage("gh:fmtlib/fmt#12.1.0")
endif()

if(NOT TARGET spdlog::spdlog)
cpmaddpackage(
    NAME
    spdlog
    VERSION
    1.17.0
    GITHUB_REPOSITORY
    "gabime/spdlog"
    OPTIONS
    "SPDLOG_FMT_EXTERNAL ON")
endif()

if(NOT TARGET CLI11::CLI11)
cpmaddpackage("gh:CLIUtils/CLI11@2.6.1")
endif()

add_subdirectory(tcp-client)
add_subdirectory(tcp-server)

add_subdirectory(local-stream-client)
add_subdirectory(local-stream-server)

add_subdirectory(udp-client)
add_subdirectory(udp-server)

add_subdirectory(local-datagram-client)
add_subdirectory(local-datagram-server)

add_subdirectory(can-publisher)
add_subdirectory(can-subscriber)

add_subdirectory(tcp-splice)

add_subdirectory(memory-map)

add_subdirectory(shared-memory-consumer)
add_subdirectory(shared-memory-producer)

add_subdirectory(timer)

add_subdirectory(poll)
add_subdirectory(select)

add_subdirectory(fifo)

add_subdirectory(message-queue-publisher)
add_subdirectory(message-queue-subscriber)