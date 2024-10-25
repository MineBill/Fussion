add_subdirectory(Vendor)

add_executable(Editor
    Source/EditorApplication.cpp
    Source/EditorApplication.h
    Source/EditorCamera.cpp
    Source/EditorCamera.h
    Source/EditorPCH.h
    Source/EditorStyle.cpp
    Source/EditorStyle.h
    Source/EditorUI.cpp
    Source/EditorUI.h
    Source/EditorWindows/AssetPicker.cpp
    Source/EditorWindows/AssetPicker.h
    Source/EditorWindows/AssetRegistryViewer.cpp
    Source/EditorWindows/AssetRegistryViewer.h
    Source/EditorWindows/AssetWindows/AssetWindow.cpp
    Source/EditorWindows/AssetWindows/AssetWindow.h
    Source/EditorWindows/AssetWindows/MaterialWindow.cpp
    Source/EditorWindows/AssetWindows/MaterialWindow.h
    Source/EditorWindows/AssetWindows/Texture2DWindow.cpp
    Source/EditorWindows/AssetWindows/Texture2DWindow.h
    Source/EditorWindows/ConsoleWindow.cpp
    Source/EditorWindows/ConsoleWindow.h
    Source/EditorWindows/ContentBrowser.cpp
    Source/EditorWindows/ContentBrowser.h
    Source/EditorWindows/EditorWindow.h
    Source/EditorWindows/InspectorWindow.cpp
    Source/EditorWindows/InspectorWindow.h
    Source/EditorWindows/RendererReport.cpp
    Source/EditorWindows/RendererReport.h
    Source/EditorWindows/SceneTreeWindow.cpp
    Source/EditorWindows/SceneTreeWindow.h
    Source/EditorWindows/ScriptsInspector.cpp
    Source/EditorWindows/ScriptsInspector.h
    Source/EditorWindows/ViewportWindow.cpp
    Source/EditorWindows/ViewportWindow.h
    Source/ImGuiHelpers.cpp
    Source/ImGuiHelpers.h
    Source/ImGuizmo.cpp
    Source/ImGuizmo.h
    Source/Layers/Editor.cpp
    Source/Layers/Editor.h
    Source/Layers/ImGuiBuild.cpp
    Source/Layers/ImGuiLayer.cpp
    Source/Layers/ImGuiLayer.h
    Source/Layers/ProjectCreatorLayer.cpp
    Source/Layers/ProjectCreatorLayer.h
    Source/Project/EditorAssetManager.cpp
    Source/Project/EditorAssetManager.h
    Source/Project/EditorAssetMetadata.h
    Source/Project/Project.cpp
    Source/Project/Project.h
    Source/SceneRenderer.cpp
    Source/SceneRenderer.h
    Source/Serialization/AssetSerializer.h
    Source/Serialization/MeshSerializer.cpp
    Source/Serialization/MeshSerializer.h
    Source/Serialization/ShaderSerializer.cpp
    Source/Serialization/ShaderSerializer.h
    Source/Serialization/TextureSerializer.cpp
    Source/Serialization/TextureSerializer.h
    Source/Serialization/mikktspace.c
    Source/Serialization/mikktspace.h
    Source/Serialization/tiny_gltf.h
    Source/Serialization/tiny_gltf_build.cpp
    Source/Undo.cpp
    Source/Undo.h
    Source/main.cpp
)

# Prevents PCH from being included in a C files, which result results in weird errors, as the PCH contains C++ headers.
set_source_files_properties(Source/Serialization/mikktspace.c PROPERTIES SKIP_PRECOMPILE_HEADERS ON)

target_link_libraries(Editor PRIVATE
    Fussion::Core
    imgui
    meshoptimizer
)

set_property(TARGET Editor PROPERTY CXX_STANDARD 23)

target_precompile_headers(Editor PRIVATE Source/EditorPCH.h)

target_include_directories(Editor PRIVATE Source)

target_compile_definitions(Editor PRIVATE
    "FSN_MATH_USE_GLOBALLY"
    "FSN_CORE_USE_GLOBALLY"
)

b_embed(Editor Assets/Icons/logo_32.png)
b_embed(Editor Assets/Icons/logo_64.png)
b_embed(Editor Assets/Icons/logo_512.png)

function (copy_dlls target)
    if (WIN32)
        add_custom_command (
            TARGET "${target}" POST_BUILD
            COMMAND "${CMAKE_COMMAND}" -E copy -t "$<TARGET_FILE_DIR:${target}>"
            "$<TARGET_RUNTIME_DLLS:${target}>" USES_TERMINAL COMMAND_EXPAND_LISTS
        )
    endif ()
endfunction ()

copy_dlls(Editor)