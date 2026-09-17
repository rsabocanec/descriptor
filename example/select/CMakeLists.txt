cmake_minimum_required(VERSION 3.22)
project(select)

set(CMAKE_CXX_STANDARD 23)

add_executable(select main.cpp)

add_dependencies(select descriptor)

target_include_directories(select PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(select PRIVATE descriptor)
