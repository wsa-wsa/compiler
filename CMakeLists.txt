cmake_minimum_required(VERSION 3.5.0)
project(stl_debug VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_BUILD_TYPE "Debug")

INCLUDE_DIRECTORIES(
    include
)

add_subdirectory(src)

