add_executable(FussionTests
    Source/Core/Benchmarks.cpp
    Source/Core/Core_Tests.cpp
    Source/Core/DynamicArray_Tests.cpp
    Source/Core/Math_Tests.cpp
    Source/Core/String_Tests.cpp
    Source/Serialization/Json.cpp
    Source/Serialization/Yaml.cpp
    Source/Common.cpp
    Source/Common.h
    Source/main.cpp
)

target_link_libraries(FussionTests
    PRIVATE
    Fussion::Core
    Catch2::Catch2
)

GroupSourcesByFolder(FussionTests "Fussion/Tests")