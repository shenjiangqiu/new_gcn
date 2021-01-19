cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_STANDARD 17)

add_executable(transformer main.cc transformer.cc)
target_include_directories(transformer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})