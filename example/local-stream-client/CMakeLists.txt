cmake_minimum_required(VERSION 3.28)
project(local-stream-client)

set(CMAKE_CXX_STANDARD 23)

add_executable(local-stream-client main.cpp)

add_dependencies(local-stream-client descriptor)

target_link_libraries(local-stream-client PRIVATE descriptor)

target_include_directories(local-stream-client PRIVATE ${CMAKE_SOURCE_DIR})