include(FetchContent)

function(InstallWGPUNative)
    # Set version for the wgpu-native library
    set(WGPU_VERSION "v22.1.0.5-custom")


    # Determine the URL and SHA256 hash based on the platform
    if (WIN32 AND CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(WGPU_URL "https://github.com/MineBill/wgpu-native/releases/download/${WGPU_VERSION}/wgpu-windows-x86_64-msvc-release.zip")
        set(WGPU_HASH "afc9c5e9d4fe1625e2e1c906b8b7ef950b17605e1ada25d8d834adeb928eeb53")
    elseif (UNIX AND CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64")
        set(WGPU_URL "https://github.com/MineBill/wgpu-native/releases/download/${WGPU_VERSION}/wgpu-linux-x86_64-release.zip")
        set(WGPU_HASH "d28938ff05d641e61872953010e9a960cac5c60fa0fbfe70a9368399c7e423b7")
    else ()
        message(FATAL_ERROR "Unsupported platform")
    endif ()

    # Download and extract the wgpu-native library
    FetchContent_Declare(
        wgpu_native
        URL ${WGPU_URL}
        URL_HASH SHA256=${WGPU_HASH}
    )
    FetchContent_MakeAvailable(wgpu_native)

    # Create an imported target for wgpu-native
    add_library(wgpu_native STATIC IMPORTED GLOBAL)

    # Set the properties for the imported target based on the platform
    if (WIN32)
        if (BUILD_SHARED_LIBS)
            set_target_properties(wgpu_native PROPERTIES
                IMPORTED_LOCATION "${wgpu_native_SOURCE_DIR}/lib/wgpu_native.dll"
                IMPORTED_IMPLIB "${wgpu_native_SOURCE_DIR}/lib/wgpu_native.dll.lib"
            )
        else ()
            set_target_properties(wgpu_native PROPERTIES
                IMPORTED_LOCATION "${wgpu_native_SOURCE_DIR}/lib/wgpu_native.lib"
            )
        endif ()
        # Set system libraries to link with wgpu_native
        target_link_libraries(wgpu_native INTERFACE
            Advapi32
            bcrypt
            d3dcompiler
            NtDll
            User32
            Userenv
            WS2_32
            Gdi32
            Opengl32
            OleAut32
            Ole32
        )
    elseif (UNIX)
        if (BUILD_SHARED_LIBS)
            set_target_properties(wgpu_native PROPERTIES
                IMPORTED_LOCATION "${wgpu_native_SOURCE_DIR}/lib/libwgpu_native.so"
            )
        else ()
            set_target_properties(wgpu_native PROPERTIES
                IMPORTED_LOCATION "${wgpu_native_SOURCE_DIR}/lib/libwgpu_native.a"
            )
        endif ()
        # Set system libraries to link with wgpu_native
        target_link_libraries(wgpu_native INTERFACE
            dl
            pthread
        )
    endif ()

    # Specify the include directories
    target_include_directories(wgpu_native INTERFACE
        "${wgpu_native_SOURCE_DIR}/include"
        "${wgpu_native_SOURCE_DIR}/include/webgpu"
    )

    # Define installation steps
    install(DIRECTORY "${wgpu_native_SOURCE_DIR}/include/"
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/webgpu"
        FILES_MATCHING PATTERN "*.h")

    if (WIN32)
        if (BUILD_SHARED_LIBS)
            install(FILES "${wgpu_native_SOURCE_DIR}/lib/wgpu_native.dll"
                DESTINATION ${CMAKE_INSTALL_BINDIR})
            install(FILES "${wgpu_native_SOURCE_DIR}/lib/wgpu_native.pdb"
                DESTINATION ${CMAKE_INSTALL_BINDIR})
            install(FILES "${wgpu_native_SOURCE_DIR}/lib/wgpu_native.dll.lib"
                DESTINATION ${CMAKE_INSTALL_LIBDIR})
        else ()
            install(FILES "${wgpu_native_SOURCE_DIR}/lib/wgpu_native.lib"
                DESTINATION ${CMAKE_INSTALL_LIBDIR})
        endif ()
    elseif (UNIX)
        if (BUILD_SHARED_LIBS)
            install(FILES "${wgpu_native_SOURCE_DIR}/lib/libwgpu_native.so"
                DESTINATION ${CMAKE_INSTALL_BINDIR})
        else ()
            install(FILES "${wgpu_native_SOURCE_DIR}/lib/libwgpu_native.a"
                DESTINATION ${CMAKE_INSTALL_LIBDIR})
        endif ()
    endif ()

    # Testing - using CTest
    enable_testing()

    # Generate the test source code
    file(GENERATE OUTPUT "${CMAKE_BINARY_DIR}/test_wgpu.c" CONTENT
        "
#include <webgpu/wgpu.h>
#include <stdio.h>
int main() {
    WGPUInstance instance = wgpuCreateInstance(NULL);
    if (instance) {
        printf(\"wgpuCreateInstance exists!\\n\");
    } else {
        printf(\"Failed to create WGPUInstance.\\n\");
    }
    return 0;
}
")

    # Add the test executable using the generated source file
    add_executable(wgpu_test "${CMAKE_BINARY_DIR}/test_wgpu.c")
    target_link_libraries(wgpu_test PRIVATE wgpu_native)

    # Add the test to CTest
    add_test(NAME wgpu_test COMMAND wgpu_test)
endfunction()