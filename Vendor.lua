package("glfw")
    add_deps("cmake")
    set_sourcedir(path.join(os.scriptdir(), "Vendor/glfw"))
    on_install(function (package)
        local configs = {}
        table.insert(configs, "-DCMAKE_BUILD_TYPE=" .. (package:debug() and "Debug" or "Release"))
        table.insert(configs, "-DBUILD_SHARED_LIBS=" .. (package:config("shared") and "ON" or "OFF"))
        table.insert(configs, "-DGLFW_BUILD_TESTS=OFF")
        table.insert(configs, "-DGLFW_BUILD_EXAMPLES=OFF")
        table.insert(configs, "-DGLFW_BUILD_DOCS=OFF")
        if is_plat("linux") then
            -- For linux, default to X11, Wayland(Waycrap) is very broken with imgui windows.
            -- Also, RenderDoc doesn't like Wayland.
            table.insert(configs, "-DGLFW_BUILD_X11=ON")
            table.insert(configs, "-DGLFW_BUILD_WAYLAND=OFF")
        end
        import("package.tools.cmake").install(package, configs)
    end)
package_end()
add_requires("glfw")

target "glm"
    set_kind "headeronly"
    add_includedirs("Vendor/glm", {public = true})
target_end()

package("VMA")
    add_deps("cmake")
    set_sourcedir(path.join(os.scriptdir(), "Vendor/VulkanMemoryAllocator-3.1.0"))
    on_install(function (package)
        local configs = {}
        table.insert(configs, "-DCMAKE_BUILD_TYPE=" .. (package:debug() and "Debug" or "Release"))
        table.insert(configs, "-DBUILD_SHARED_LIBS=" .. (package:config("shared") and "ON" or "OFF"))
        import("package.tools.cmake").install(package, configs)
    end)
package_end()
add_requires("VMA")

target("AngelScript")
    set_kind("static")
    set_languages("c++20")
    set_version("2.36.1")
    set_group("Vendor")

    add_files(
        "Engin5/Vendor/angelscript/sdk/angelscript/source/**.cpp",
        "Engin5/Vendor/angelscript/sdk/add_on/**.cpp|autowrapper/generator/generateheader.cpp"
    )
    add_headerfiles(
        "Engin5/Vendor/angelscript/sdk/angelscript/source/**.h",
        "Engin5/Vendor/angelscript/sdk/add_on/**.h"
    )
    add_sysincludedirs("Engin5/Vendor/angelscript/sdk/angelscript/include", {public = true})
    add_sysincludedirs("Engin5/Vendor/angelscript/sdk/add_on/", {public = true})

    if is_plat("windows") then
        add_files("Engin5/Vendor/angelscript/sdk/angelscript/source/as_callfunc_x64_msvc_asm.asm")
        set_runtimes("MDd")
    end
target_end()

target "magic_enum"
    set_kind "headeronly"
    add_sysincludedirs("Vendor/magic_enum/include", {public = true})
    set_group("Vendor")
target_end()

target "Reflect"
    set_kind("static")
    set_languages("c++20")
    set_group("Vendor")

    add_files("Engin5/Vendor/Reflect/Reflect/src/**.cpp")
    add_headerfiles("Engin5/Vendor/Reflect/Reflect/inc/**.h")
    add_sysincludedirs("Engin5/Vendor/Reflect/Reflect/inc", {public = true})
    add_defines("REFLECT_TYPE_INFO_ENABLED", {public = true})

    set_policy("build.fence", true)

    add_rules("CompilerFlags")

    if is_plat("windows") then
        set_runtimes("MDd")
        add_defines("OS_WINDOWS")
    elseif is_plat("linux") then
        add_defines("OS_LINUX")
    elseif is_plat("macos") then
        add_defines("OS_MACOS")
    end
target_end()

target("kdl")
    set_kind("static")
    set_group("vendor")
    set_languages("c++20", "c11")

    add_files(
        "Editor/Vendor/ckdl/src/**.c",
        "Editor/Vendor/ckdl/bindings/cpp/src/kdlpp.cpp"
    )

    add_headerfiles("Editor/Vendor/ckdl/src/**.h")

    add_sysincludedirs(
        "Editor/Vendor/ckdl/include",
        "Editor/Vendor/ckdl/bindings/cpp/include",
        {public = true}
    )

    add_defines("KDLPP_STATIC_LIB", "KDL_STATIC_LIB", {public = true})

    add_rules("CompilerFlags")
    if is_plat("windows") then
        set_runtimes("MDd")
    end
target_end()

if is_plat("linux") then
    add_requires("spirv-cross")
end