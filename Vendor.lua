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

package("rttr")
    add_deps("cmake")
    set_sourcedir(path.join(os.scriptdir(), "Vendor/rttr-0.9.6"))
    on_install(function (package)
        local configs = {}
        table.insert(configs, "-DCMAKE_BUILD_TYPE=" .. (package:debug() and "Debug" or "Release"))
        table.insert(configs, "-DBUILD_SHARED_LIBS=" .. (package:config("shared") and "ON" or "OFF"))
        import("package.tools.cmake").install(package, configs)
    end)
package_end()
add_requires("rttr")

target("glm", function()
    set_kind "headeronly"
    set_group "Vendor"

    add_includedirs("Vendor/glm", {public = true})
end)

target("AngelScript", function()
    set_kind "static"
    set_languages "c++20"
    set_version "2.36.1"
    set_group "Vendor"

    add_files(
        "Fussion/Vendor/angelscript/sdk/angelscript/source/**.cpp",
        "Fussion/Vendor/angelscript/sdk/add_on/**.cpp|autowrapper/generator/generateheader.cpp"
    )
    add_headerfiles(
        "Fussion/Vendor/angelscript/sdk/angelscript/source/**.h",
        "Fussion/Vendor/angelscript/sdk/add_on/**.h"
    )
    add_sysincludedirs("Fussion/Vendor/angelscript/sdk/angelscript/include", {public = true})
    add_sysincludedirs("Fussion/Vendor/angelscript/sdk/add_on/", {public = true})

    if is_plat("windows") then
        add_files "Fussion/Vendor/angelscript/sdk/angelscript/source/as_callfunc_x64_msvc_asm.asm"
        set_runtimes "MDd"
    end
end)

target("magic_enum", function()
    set_kind "headeronly"
    set_group "Vendor"

    add_sysincludedirs("Vendor/magic_enum/include", {public = true})
end)

target("kdl", function()
    set_kind "static"
    set_group "Vendor"
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
end)

if is_plat("linux") then
    add_requires("spirv-cross")
end