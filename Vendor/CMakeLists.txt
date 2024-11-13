include(wgpu-native.cmake)

add_subdirectory(Catch2-3.6.0 SYSTEM)
add_subdirectory(cpptrace-0.6.3 SYSTEM)
add_subdirectory(fmt-11.0.2 SYSTEM)

set(GLFW_BUILD_DOCS OFF)
set(GLFW_BUILD_EXAMPLES OFF)
set(GLFW_BUILD_TESTS OFF)
if (UNIX)
    set(GLFW_BUILD_X11 ON)
    set(GLFW_BUILD_WAYLAND ON)
endif ()
add_subdirectory(glfw SYSTEM)

set(GLM_ENABLE_CXX_20 ON)
add_subdirectory(glm SYSTEM)

add_subdirectory(magic_enum SYSTEM)

set(TINYGLTF_BUILD_LOADER_EXAMPLE OFF)
add_subdirectory(tinygltf-2.8.23 SYSTEM)

set(B_EMBED_SILENCE_DEVMODE_WARNING ON)
add_subdirectory(embed SYSTEM)

set(YAML_CPP_STATIC_DEFINE ON)
set(YAML_CPP_BUILD_TOOLS OFF)
set(YAML_CPP_BUILD_TESTS OFF)
add_subdirectory(yaml-cpp SYSTEM)

add_subdirectory(meta.hpp SYSTEM)

add_subdirectory(rgbcx)

function(InstallAngelscriptAddons)
    add_library(angelscript_addons
        STATIC
        ../Fussion/Vendor/angelscript/sdk/add_on/scriptbuilder/scriptbuilder.h
        ../Fussion/Vendor/angelscript/sdk/add_on/scriptbuilder/scriptbuilder.cpp

        ../Fussion/Vendor/angelscript/sdk/add_on/scriptarray/scriptarray.h
        ../Fussion/Vendor/angelscript/sdk/add_on/scriptarray/scriptarray.cpp

        ../Fussion/Vendor/angelscript/sdk/add_on/scriptstdstring/scriptstdstring.h
        ../Fussion/Vendor/angelscript/sdk/add_on/scriptstdstring/scriptstdstring.cpp
        ../Fussion/Vendor/angelscript/sdk/add_on/scriptstdstring/scriptstdstring_utils.cpp

        ../Fussion/Vendor/angelscript/sdk/add_on/scriptany/scriptany.h
        ../Fussion/Vendor/angelscript/sdk/add_on/scriptany/scriptany.cpp
    )

    target_include_directories(angelscript_addons
        SYSTEM PUBLIC
        ../Fussion/Vendor/angelscript/sdk/add_on
    )

    target_link_libraries(
        angelscript_addons
        PUBLIC
        angelscript
    )
endfunction()


function(InstallAATC)
    add_library(argparse INTERFACE)
    target_include_directories(argparse INTERFACE argparse/include)

    add_library(aatc STATIC
        aatc/source/aatc_container_map.cpp
        aatc/source/aatc_container_listing_heavy.hpp
        aatc/source/aatc_container_listing.hpp
        aatc/source/aatc_container_list.hpp
        aatc/source/aatc_container_list.cpp
        aatc/source/aatc_container_deque.hpp
        aatc/source/aatc_container_deque.cpp
        aatc/source/aatc_config.hpp
        aatc/source/aatc_common.hpp
        aatc/source/aatc_common.cpp
        aatc/source/aatc.hpp
        aatc/source/aatc.cpp
        aatc/source/aatc_container_shared.cpp
        aatc/source/aatc_container_set.hpp
        aatc/source/aatc_container_set.cpp
        aatc/source/aatc_container_mapped_templated_shared_method.hpp
        aatc/source/aatc_container_mapped_templated_shared.hpp
        aatc/source/aatc_container_map.hpp
        aatc/source/aatc_container_templated_shared_method.hpp
        aatc/source/aatc_container_templated_shared.hpp
        aatc/source/aatc_container_shared.hpp
        aatc/source/aatc_container_tempspec_shared_method.hpp
        aatc/source/aatc_container_tempspec_shared.hpp
        aatc/source/aatc_container_unordered_map.cpp
        aatc/source/aatc_container_unordered_map.hpp
        aatc/source/aatc_templatemagic.hpp
        aatc/source/aatc_serializer.hpp
        aatc/source/aatc_serializer.cpp
        aatc/source/aatc_registration.cpp
        aatc/source/aatc_hash.hpp
        aatc/source/aatc_hash.cpp
        aatc/source/aatc_enginestorage.hpp
        aatc/source/aatc_enginestorage.cpp
        aatc/source/aatc_container_vector.hpp
        aatc/source/aatc_container_vector.cpp
        aatc/source/aatc_container_unordered_set.hpp
        aatc/source/aatc_container_unordered_set.cpp
    )

    target_include_directories(aatc SYSTEM INTERFACE aatc/source)
    target_link_libraries(aatc PRIVATE angelscript)
endfunction()

function(InstallSlang)
    # https://github.com/shader-slang/slang/releases/download/v${SLANG_VERSION}/slang-${SLANG_VERSION}-windows-x86_64.zip
    set(SLANG_VERSION 2024.14.2)
    if (WIN32)
        set(SLANG_OS "windows")
        set(SLANG_URL_HASH "68b8e480353707cfd043b74939d9f6a12183c9a8dbd09dc2ef0c728de1212c03")
    endif ()
    if (UNIX)
        set(SLANG_OS "linux")
        set(SLANG_URL_HASH "48c951efd96bd547d96a8a3a8d9160ab5489b8dd5ed71de0e16e135e02a03f6d")
    endif ()
    CPMAddPackage(
        NAME Slang
        VERSION ${SLANG_VERSION}
        URL https://github.com/shader-slang/slang/releases/download/v${SLANG_VERSION}/slang-${SLANG_VERSION}-${SLANG_OS}-x86_64.zip
        URL_HASH SHA256=${SLANG_URL_HASH}
    )

    if (Slang_ADDED)
        add_library(Slang SHARED IMPORTED GLOBAL)

        if (WIN32)
            set_target_properties(Slang PROPERTIES
                IMPORTED_LOCATION "${Slang_SOURCE_DIR}/bin/slang.dll"
                IMPORTED_IMPLIB "${Slang_SOURCE_DIR}/lib/slang.lib"
            )
        elseif (UNIX)
            set_target_properties(Slang PROPERTIES
                IMPORTED_LOCATION "${Slang_SOURCE_DIR}/lib/libslang.so"
            )
        endif ()

        target_include_directories(Slang SYSTEM INTERFACE "${Slang_SOURCE_DIR}/include")
    endif ()
endfunction()

function(InstallLinuxDeps)
    if (LINUX)
        add_subdirectory(libsigcplusplus SYSTEM)
        set(BUILD_TESTING OFF)
        add_subdirectory(dbus-cxx SYSTEM)
    endif ()
endfunction()

function(InstallSTB)
    add_library(stb
        STATIC
        stb/build.c
    )

    target_include_directories(stb SYSTEM PUBLIC stb/include)
endfunction()

function(InstallMikktspace)
    add_library(mikktspace
        STATIC
        mikktspace/mikktspace.c
    )

    target_include_directories(mikktspace SYSTEM PUBLIC mikktspace/)
endfunction()

InstallAATC()
InstallWGPUNative()
InstallSlang()
InstallSTB()
InstallMikktspace()
InstallAngelscriptAddons()

InstallLinuxDeps()

set_target_properties(
    glm
    glfw
    fmt
    magic_enum
    tinygltf
    aatc
    argparse
    cpptrace-lib
    TracyClient
    angelscript
    Jolt
    wgpu_native
    Slang
    yaml-cpp
    meta.hpp
    Catch2
    PROPERTIES FOLDER
    Vendor
)