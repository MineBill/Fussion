cmake_minimum_required(VERSION 3.29.0)
project(Fussion
    VERSION 0.1
    LANGUAGES C CXX)

include(cmake/CPM.cmake)
include(cmake/CompilerWarnings.cmake)
include(cmake/Cache.cmake)
include(cmake/StandardOptions.cmake)

TryEnableCache()

add_subdirectory(Fussion/Vendor)
add_subdirectory(Vendor)

add_subdirectory(HeaderTool)
add_subdirectory(Fussion)
add_subdirectory(Editor)