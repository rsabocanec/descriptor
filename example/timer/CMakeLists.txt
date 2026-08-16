cmake_minimum_required(VERSION 3.28.3)

set(CMAKE_CXX_STANDARD 23)

add_executable(timer main.cpp)
add_dependencies(timer descriptor)

target_link_libraries(timer PRIVATE descriptor)

target_include_directories(timer PRIVATE ${CMAKE_SOURCE_DIR})
