add_library(Fussion STATIC
    Source/FussionPCH.h
    Source/e5pch.cpp
    Source/Fussion/Window.h
    Source/Fussion/Events/MouseEvents.h
    Source/Fussion/Events/KeyboardEvents.h
    Source/Fussion/Events/Event.h
    Source/Fussion/Events/ApplicationEvents.h
    Source/Fussion/Util/Util.cpp
    Source/Fussion/Util/TextureLoader.h
    Source/Fussion/Util/TextureLoader.cpp
    Source/Fussion/Util/stb_image_write.h
    Source/Fussion/Util/stb_image.h
    Source/Fussion/Util/SimpleParser.h
    Source/Fussion/Util/SimpleParser.cpp
    Source/Fussion/Util/SimpleLexer.h
    Source/Fussion/Util/SimpleLexer.cpp
    Source/Fussion/Assets/Texture2D.h
    Source/Fussion/Assets/Texture2D.cpp
    Source/Fussion/Assets/ShaderAsset.h
    Source/Fussion/Assets/ShaderAsset.cpp
    Source/Fussion/Assets/PbrMaterial.h
    Source/Fussion/Assets/PbrMaterial.cpp
    Source/Fussion/Assets/Model.h
    Source/Fussion/Assets/Model.cpp
    Source/Fussion/Assets/AssetSettings.h
    Source/Fussion/Assets/AssetRef.h
    Source/Fussion/Assets/AssetRef.cpp
    Source/Fussion/Assets/AssetManagerBase.h
    Source/Fussion/Assets/AssetManager.h
    Source/Fussion/Assets/AssetManager.cpp
    Source/Fussion/Assets/Asset.h
    Source/Fussion/Assets/Asset.cpp
    Source/Fussion/Core/Ref.h
    Source/Fussion/Core/Ref.cpp
    Source/Fussion/Core/Mem.h
    Source/Fussion/Core/Mem.cpp
    Source/Fussion/Core/Maybe.h
    Source/Fussion/Core/Layer.h
    Source/Fussion/Core/DynamicArray.h
    Source/Fussion/Core/Delegate.h
    Source/Fussion/Core/Core.h
    Source/Fussion/Core/Concepts.h
    Source/Fussion/Core/BitFlags.h
    Source/Fussion/Core/Application.h
    Source/Fussion/Core/Application.cpp
    Source/Fussion/Core/StringUtils.h
    Source/Fussion/Core/StringUtils.cpp
    Source/Fussion/Core/String.h
    Source/Fussion/Core/String.cpp
    Source/Fussion/Core/SmallVector.h
    Source/Fussion/Core/Slice.h
    Source/Fussion/Core/Result.h
    Source/Fussion/Core/Time.h
    Source/Fussion/Core/Time.cpp
    Source/Fussion/Core/ThreadProtected.h
    Source/Fussion/Core/Uuid.cpp
    Source/Fussion/Core/Types.h
    Source/Fussion/Core/Uuid.h
    Source/Fussion/Debug/Debug.h
    Source/Fussion/Debug/Debug.cpp
    Source/Fussion/Serialization/BinarySerializer.h
    Source/Fussion/Serialization/BinarySerializer.cpp
    Source/Fussion/Serialization/Serializer.h
    Source/Fussion/Serialization/Serializer.cpp
    Source/Fussion/Serialization/nlohmann_json.natvis
    Source/Fussion/Serialization/JsonSerializer.h
    Source/Fussion/Serialization/JsonSerializer.cpp
    Source/Fussion/Serialization/json.hpp
    Source/Fussion/Serialization/Json.h
    Source/Fussion/Serialization/Json.cpp
    Source/Fussion/Serialization/ISerializable.h
    Source/Fussion/Serialization/ISerializable.cpp
    Source/Fussion/Reflection/ReflectionRegistry.h
    Source/Fussion/Reflection/ReflectionRegistry.cpp
    Source/Fussion/Reflection/PrivateReflection.h
    Source/Fussion/Reflection/Attributes.h
    Source/Fussion/OS/System.h
    Source/Fussion/OS/FileWatcher.h
    Source/Fussion/OS/FileSystem.h
    Source/Fussion/OS/FileSystem.cpp
    Source/Fussion/OS/Dialog.h
    Source/Fussion/OS/Clock.h
    Source/Fussion/OS/Clock.cpp
    Source/Fussion/OS/Args.h
    Source/Fussion/OS/Args.cpp
    Source/Fussion/meta.hpp/meta_uvalue.hpp
    Source/Fussion/Physics/Physics.h
    Source/Fussion/Physics/Physics.cpp
    Source/Fussion/Scripting/AttributeParser.h
    Source/Fussion/Scripting/AttributeParser.cpp
    Source/Fussion/Scripting/Attribute.h
    Source/Fussion/Scripting/ScriptingEngineTypes.cpp
    Source/Fussion/Scripting/ScriptingEngine.h
    Source/Fussion/Scripting/ScriptingEngine.cpp
    Source/Fussion/Scripting/ScriptBase.h
    Source/Fussion/Scripting/ScriptBase.cpp
    Source/Fussion/Scripting/ScriptAssembly.h
    Source/Fussion/Scripting/ScriptAssembly.cpp
    Source/Fussion/Scripting/Script.h
    Source/Fussion/Scripting/Script.cpp
    Source/Fussion/Rendering/UniformBuffer.h
    Source/Fussion/Rendering/RenderTypes.h
    Source/Fussion/Rendering/RenderTypes.cpp
    Source/Fussion/Rendering/Renderer.h
    Source/Fussion/Rendering/Renderer.cpp
    Source/Fussion/Scripting/Attributes/RangeAttribute.h
    Source/Fussion/Scripting/Attributes/EditableAttribute.h
    Source/Fussion/Rendering/Pipelines/TonemappingPipeline.h
    Source/Fussion/Rendering/Pipelines/TonemappingPipeline.cpp
    Source/Fussion/Rendering/Pipelines/SSAOBlur.h
    Source/Fussion/Rendering/Pipelines/SSAOBlur.cpp
    Source/Fussion/Rendering/Pipelines/IrradianceIBLGenerator.h
    Source/Fussion/Rendering/Pipelines/IrradianceIBLGenerator.cpp
    Source/Fussion/Rendering/Pipelines/CubeMapSkybox.h
    Source/Fussion/Rendering/Pipelines/CubeMapSkybox.cpp
    Source/Fussion/Scripting/AngelDumper.cpp
    Source/Fussion/Scripting/AngelDumper.h
    Source/Fussion/Scene/Entity.h
    Source/Fussion/Scene/Entity.cpp
    Source/Fussion/Scene/Scene.h
    Source/Fussion/Scene/Scene.cpp
    Source/Fussion/Scene/Forward.h
    Source/Fussion/Scene/Components/BaseComponents.cpp
    Source/Fussion/Scene/Component.h
    Source/Fussion/Scene/Components/BaseComponents.h
    Source/Fussion/Fussion.h
    Source/Fussion/Input/Input.cpp
    Source/Fussion/Image.h
    Source/Fussion/Input/Input.h
    Source/Fussion/Input/Keys.h
    Source/Fussion/Scene/Components/ScriptComponent.h
    Source/Fussion/Scene/Components/ScriptComponent.cpp
    Source/Fussion/Scene/Components/PhysicsComponents.h
    Source/Fussion/Scene/Components/PhysicsComponents.cpp
    Source/Fussion/Scene/Components/MeshRenderer.h
    Source/Fussion/Scene/Components/MeshRenderer.cpp
    Source/Fussion/Scene/Components/DirectionalLight.h
    Source/Fussion/Scene/Components/DirectionalLight.cpp
    Source/Fussion/Scene/Components/Camera.h
    Source/Fussion/Scene/Components/Camera.cpp
    Source/Fussion/GPU/Enums.h
    Source/Fussion/GPU/EnumConversions.h
    Source/Fussion/GPU/EnumConversions.cpp
    Source/Fussion/GPU/glfw3webgpu.h
    Source/Fussion/GPU/GPU.cpp
    Source/Fussion/GPU/GPU.h
    Source/Fussion/GPU/Utils.h
    Source/Fussion/GPU/Utils.cpp
    Source/Fussion/GPU/ShaderProcessor.h
    Source/Fussion/GPU/ShaderProcessor.cpp
    Source/Fussion/Math/VectorCommon.h
    Source/Fussion/Math/Vector4.h
    Source/Fussion/Math/Vector4.cpp
    Source/Fussion/Math/Vector3.h
    Source/Fussion/Math/Vector3.cpp
    Source/Fussion/Math/Vector2.h
    Source/Fussion/Math/Vector2.cpp
    Source/Fussion/Math/Rect.h
    Source/Fussion/Math/Rect.cpp
    Source/Fussion/Math/Math.h
    Source/Fussion/Math/Math.cpp
    Source/Fussion/Math/Color.h
    Source/Fussion/Math/Color.cpp
    Source/Fussion/Math/BoundingBox.h
    Source/Fussion/Math/BoundingBox.cpp
    Source/Fussion/Log/Formatters.h
    Source/Fussion/Log/FileSink.h
    Source/Fussion/Log/FileSink.cpp
    Source/Fussion/Log/Log.cpp
    Source/Fussion/Log/Log.h

    Impl/GlfwWindow.cpp
    Impl/GlfwWindow.h
    Source/Fussion/GPU/glfw3webgpu.c
    Source/Fussion/GPU/glfw3webgpu.h

    ${CMAKE_BINARY_DIR}/ReflectionRegistry_Generated.cpp
)

get_target_property(Fussion_SOURCES Fussion SOURCES)

set(Fussion_HEADERS)

foreach (file ${Fussion_SOURCES})
    # Check if the file has a .h extension
    if (file MATCHES "\\.(h|hpp)$" AND file MATCHES "^Source" AND NOT file MATCHES "meta\\.hpp")
        list(APPEND Fussion_HEADERS ${file})
    endif ()
endforeach ()

target_sources(Fussion PRIVATE
    Vendor/angelscript/sdk/add_on/scriptbuilder/scriptbuilder.h
    Vendor/angelscript/sdk/add_on/scriptbuilder/scriptbuilder.cpp

    Vendor/angelscript/sdk/add_on/scriptarray/scriptarray.h
    Vendor/angelscript/sdk/add_on/scriptarray/scriptarray.cpp

    Vendor/angelscript/sdk/add_on/scriptstdstring/scriptstdstring.h
    Vendor/angelscript/sdk/add_on/scriptstdstring/scriptstdstring.cpp
    Vendor/angelscript/sdk/add_on/scriptstdstring/scriptstdstring_utils.cpp

    Vendor/angelscript/sdk/add_on/scriptany/scriptany.h
    Vendor/angelscript/sdk/add_on/scriptany/scriptany.cpp
)

set_source_files_properties(Source/Fussion/GPU/glfw3webgpu.c PROPERTIES SKIP_PRECOMPILE_HEADERS ON)

add_library(Fussion::Core ALIAS Fussion)

add_dependencies(Fussion HeaderTool)

target_include_directories(Fussion PUBLIC # TODO: Should this be public?
    Vendor/angelscript/sdk/add_on
)

ApplyCommonCompilerOptions(Fussion)

target_precompile_headers(Fussion PRIVATE Source/FussionPCH.h)

target_link_libraries(Fussion PUBLIC
    glm::glm
    glfw
    fmt::fmt
    magic_enum::magic_enum
    tinygltf
    aatc
    argparse
    cpptrace::cpptrace
    TracyClient
    angelscript
    Jolt
    wgpu_native
    Slang
    yaml-cpp::yaml-cpp
    meta.hpp::meta.hpp
)

target_include_directories(Fussion
    PUBLIC
    Source
    PRIVATE
    Source/Fussion
    Impl
)

target_compile_definitions(Fussion PUBLIC
    GLM_ENABLE_EXPERIMENTAL
    USE_ASSERTIONS
    TRACY_ENABLE
    TRACY_ON_DEMAND
)

if (WIN32)
    target_compile_definitions(Fussion PUBLIC OS_WINDOWS)
    target_sources(Fussion PRIVATE
        Platform/Windows/WindowsDialog.cpp
        Platform/Windows/WindowsFileWatcher.cpp
        Platform/Windows/WindowsSystem.cpp
    )
endif ()

if (UNIX)
    set(LINUX_DEFINES OS_LINUX)
    if (GLFW_BUILD_WAYLAND)
        message("==================== Building with wayland glfw so adding extra thing")
        list(APPEND LINUX_DEFINES FUSSION_GLFW_WAYLAND)
    endif ()
    target_compile_definitions(Fussion PUBLIC ${LINUX_DEFINES})
    target_sources(Fussion PRIVATE
        Platform/Linux/LinuxDialog.cpp
        Platform/Linux/LinuxFileWatcher.cpp
        Platform/Linux/LinuxSystem.cpp
    )

    target_link_libraries(Fussion
        PRIVATE
        dbus-cxx
    )
endif ()

SetTargetWarnings(Fussion OFF "" "" "")

b_embed(Fussion Assets/Textures/black_texture.png)
b_embed(Fussion Assets/Textures/default_normal_map.png)
b_embed(Fussion Assets/Textures/white_texture.png)
b_embed(Fussion Assets/Textures/white_texture.hdr)

add_custom_command(
    OUTPUT ${CMAKE_BINARY_DIR}/ReflectionRegistry_Generated.cpp
    DEPENDS HeaderTool ${Fussion_HEADERS}
    COMMAND HeaderTool ${CMAKE_BINARY_DIR}
    COMMENT "Generating meta info."
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    VERBATIM
)