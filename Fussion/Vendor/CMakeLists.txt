add_subdirectory(angelscript/sdk/angelscript/projects/cmake SYSTEM)
add_subdirectory(tracy SYSTEM)
set(USE_STATIC_MSVC_RUNTIME_LIBRARY OFF)
add_subdirectory(JoltPhysics/Build SYSTEM)