cmake_minimum_required(VERSION 2.8)
project(rgbcx)

add_library(
    rgbcx
    build.cpp
    bc7decomp.cpp
    bc7enc.c
)

target_include_directories(rgbcx PRIVATE include)
target_include_directories(rgbcx INTERFACE include SYSTEM)

add_executable(
    rgbcx_cli
    cli/test.cpp
    cli/lodepng.cpp
)

target_link_libraries(rgbcx_cli PRIVATE rgbcx)