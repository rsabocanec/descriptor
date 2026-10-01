cmake_minimum_required(VERSION 3.22)
project(message-queue-publisher)

set(CMAKE_CXX_STANDARD 23)

add_executable(message-queue-publisher main.cpp)

add_dependencies(message-queue-publisher descriptor)

target_include_directories(message-queue-publisher PRIVATE ${CMAKE_SOURCE_DIR})

target_link_libraries(message-queue-publisher PRIVATE descriptor CLI11::CLI11 spdlog::spdlog fmt::fmt)
