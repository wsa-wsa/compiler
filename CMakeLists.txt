cmake_minimum_required(VERSION 3.8.0)
project(stl_debug VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)


SET(CMAKE_CXX_FLAGS_ASAN "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=undefined -fsanitize=address")

if(CMAKE_BUILD_TYPE STREQUAL "")
    set(CMAKE_BUILD_TYPE "Debug")
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

message(STATUS "BUILD TYPE : ${CMAKE_BUILD_TYPE}")

INCLUDE_DIRECTORIES(
    include
)

add_subdirectory(src)

