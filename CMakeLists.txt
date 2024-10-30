cmake_minimum_required(VERSION 3.28.0)
project(Fussion
    VERSION 0.1
    LANGUAGES C CXX)

include(cmake/CPM.cmake)
include(cmake/CompilerWarnings.cmake)
include(cmake/Cache.cmake)
include(cmake/StandardOptions.cmake)

function(ApplyCommonCompilerOptions target)
    set_property(TARGET ${target} PROPERTY CXX_STANDARD 23)
endfunction()

function(CopyRuntimeTargets target)
    if (WIN32)
        add_custom_command(
            TARGET "${target}" POST_BUILD
            COMMAND "${CMAKE_COMMAND}" -E copy -t "$<TARGET_FILE_DIR:${target}>"
            "$<TARGET_RUNTIME_DLLS:${target}>" USES_TERMINAL COMMAND_EXPAND_LISTS
        )
    endif ()
endfunction()

TryEnableCache()

add_subdirectory(Fussion/Vendor)
add_subdirectory(Vendor)

add_subdirectory(HeaderTool)
add_subdirectory(Fussion)
add_subdirectory(Editor)
if(IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/Sandbox")
    add_subdirectory(Sandbox)
endif()
