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
        table.insert(configs, "-DTINYGLTF_BUILD_LOADER_EXAMPLE=OFF")
        table.insert(configs, "-DTINYGLTF_INSTALL=OFF")
        import("package.tools.cmake").install(package, configs)
    end)
package_end()
add_requires("VMA")

package("fmt")
    add_deps("cmake")
    set_sourcedir(path.join(os.scriptdir(), "Vendor/fmt-11.0.2"))
    on_install(function (package)
        local configs = {}
        table.insert(configs, "-DCMAKE_BUILD_TYPE=" .. (package:debug() and "Debug" or "Release"))
        table.insert(configs, "-DBUILD_SHARED_LIBS=" .. (package:config("shared") and "ON" or "OFF"))
        local defines = {
            "-DFMT_DOC=OFF",
            "-DFMT_TEST=OFF",
            "-DFMT_FUZZ=OFF",
            "-DFMT_CUDA_TEST=OFF"
        }
        for k, v in pairs(defines) do
            table.insert(configs, v)
        end
        import("package.tools.cmake").install(package, configs)
    end)
package_end()
add_requires("fmt")

package("cpptrace")
    add_deps("cmake")
    set_sourcedir(path.join(os.scriptdir(), "Vendor/cpptrace-0.6.3"))
    on_install(function (package)
        local configs = {}
        table.insert(configs, "-DCMAKE_BUILD_TYPE=" .. (package:debug() and "Debug" or "Release"))
        table.insert(configs, "-DBUILD_SHARED_LIBS=" .. (package:config("shared") and "ON" or "OFF"))
		package:add("links", "cpptrace")
        if not package:config("shared") then
            package:add("defines", "CPPTRACE_STATIC_DEFINE")
        end
        import("package.tools.cmake").install(package, configs)
    end)
package_end()
add_requires("cpptrace")

package("tinygltf")
    add_deps("cmake")
    set_sourcedir(path.join(os.scriptdir(), "Vendor/tinygltf-2.8.23"))
    on_install(function (package)
        local configs = {}
        table.insert(configs, "-DCMAKE_BUILD_TYPE=" .. (package:debug() and "Debug" or "Release"))
        table.insert(configs, "-DBUILD_SHARED_LIBS=" .. (package:config("shared") and "ON" or "OFF"))
        import("package.tools.cmake").install(package, configs)
    end)
package_end()
add_requires("tinygltf")

package("Catch2")
    add_deps("cmake")
    set_sourcedir(path.join(os.scriptdir(), "Vendor/Catch2-3.6.0"))
    on_install(function (package)
        local configs = {}
        table.insert(configs, "-DCMAKE_BUILD_TYPE=" .. (package:debug() and "Debug" or "Release"))
        table.insert(configs, "-DBUILD_SHARED_LIBS=" .. (package:config("shared") and "ON" or "OFF"))
        import("package.tools.cmake").install(package, configs)
    end)
package_end()
add_requires("Catch2")

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

if is_plat("linux") then
    add_requires("spirv-cross")
end