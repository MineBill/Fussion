add_library(tree-sitter STATIC
    tree-sitter-0.22.3/lib/src/lib.c
)

target_include_directories(tree-sitter
    PUBLIC
    tree-sitter-0.22.3/lib/include
    tree-sitter-0.22.3/lib/src
)

add_library(tree-sitter-cpp STATIC
    tree-sitter-cpp/src/parser.c
    tree-sitter-cpp/src/scanner.c
)

target_include_directories(tree-sitter-cpp
    PUBLIC
    tree-sitter-cpp/bindings/c
    PRIVATE
    tree-sitter-cpp/src
)

target_link_libraries(tree-sitter-cpp PUBLIC tree-sitter)

add_executable(HeaderTool
    Source/HeaderTool.cpp
    Source/Query.cpp
    Source/Query.h
    Source/Common.h
)

set_property(TARGET HeaderTool PROPERTY CXX_STANDARD 23)

target_link_libraries(HeaderTool
    PRIVATE
    tree-sitter
    tree-sitter-cpp
    Tracy::TracyClient
)

set_target_properties(
    tree-sitter
    tree-sitter-cpp
    PROPERTIES FOLDER
    Vendor
)

GroupSourcesByFolder(HeaderTool)