add_subdirectory(meshoptimizer-0.21)

add_library(imgui STATIC
    imgui/imconfig.h
    imgui/imgui.cpp
    imgui/imgui.h
    imgui/imgui_demo.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_internal.h
    imgui/imgui_tables.cpp
    imgui/imgui_widgets.cpp
    imgui/imstb_rectpack.h
    imgui/imstb_textedit.h
    imgui/imstb_truetype.h
    imgui/misc/cpp/imgui_stdlib.cpp
    imgui/misc/cpp/imgui_stdlib.h
)

target_include_directories(imgui PUBLIC
    imgui
    imgui/misc/cpp/
)

# TODO: Can this be done any other way?
target_link_libraries(imgui PUBLIC Fussion::Core)
set_property(TARGET imgui PROPERTY CXX_STANDARD 23)
target_compile_definitions(imgui PRIVATE
    "FSN_MATH_USE_GLOBALLY"
)
