cmake_minimum_required(VERSION 3.22)
project(message-queue-subscriber)

set(CMAKE_CXX_STANDARD 23)

add_executable(message-queue-subscriber main.cpp)

add_dependencies(message-queue-subscriber descriptor)

target_include_directories(message-queue-subscriber PRIVATE ${CMAKE_SOURCE_DIR})

target_link_libraries(message-queue-subscriber PRIVATE descriptor CLI11::CLI11 spdlog::spdlog fmt::fmt)
