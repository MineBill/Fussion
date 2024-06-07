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

target "magic_enum"
    set_kind "headeronly"
    add_includedirs("Vendor/magic_enum/include", {public = true})
target_end()

target "Reflect"
    set_kind("static")
    set_languages("c++20")

    add_files("Engin5/Vendor/Reflect/Reflect/src/**.cpp")
    add_headerfiles("Engin5/Vendor/Reflect/Reflect/inc/**.h")
    add_includedirs("Engin5/Vendor/Reflect/Reflect/inc", {public = true})
    add_defines("REFLECT_TYPE_INFO_ENABLED", {public = true})

    set_policy("build.fence", true)

    if is_plat("windows") then
        set_runtimes("MDd")
    end
target_end()

if is_plat("linux") then
    add_requires("spirv-cross")
end