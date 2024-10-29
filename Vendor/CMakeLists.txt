include(wgpu-native.cmake)

add_subdirectory(Catch2-3.6.0)
add_subdirectory(cpptrace-0.6.3)
add_subdirectory(fmt-11.0.2)

set(GLFW_BUILD_DOCS OFF)
set(GLFW_BUILD_EXAMPLES OFF)
set(GLFW_BUILD_TESTS OFF)
if (UNIX)
    set(GLFW_BUILD_X11 ON)
    set(GLFW_BUILD_WAYLAND ON)
endif ()
add_subdirectory(glfw)
add_subdirectory(glm)
add_subdirectory(magic_enum)
add_subdirectory(tinygltf-2.8.23)
add_subdirectory(embed)

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

    target_include_directories(aatc INTERFACE aatc/source)
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
        add_subdirectory(libsigcplusplus)
        set(BUILD_TESTING OFF)
        add_subdirectory(dbus-cxx)
    endif ()
endfunction()

InstallAATC()
InstallWGPUNative()
InstallSlang()
InstallLinuxDeps()