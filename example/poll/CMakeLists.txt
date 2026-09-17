cmake_minimum_required(VERSION 3.22)
project(poll)

set(CMAKE_CXX_STANDARD 23)

add_executable(poll main.cpp)

add_dependencies(poll descriptor)

target_include_directories(poll PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(poll PRIVATE descriptor)
