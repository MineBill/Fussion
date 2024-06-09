package("ckdl")
    add_deps("cmake")
    set_sourcedir(path.join(os.scriptdir(), "Vendor/ckdl"))
    on_install(function (package)
        local configs = {}
        table.insert(configs, "-DCMAKE_BUILD_TYPE=" .. (package:debug() and "Debug" or "Release"))
        table.insert(configs, "-DBUILD_SHARED_LIBS=" .. (package:config("shared") and "ON" or "OFF"))
        table.insert(configs, "-DBUILD_KDLPP=ON")
        table.insert(configs, "-DBUILD_TESTS=OFF")
        import("package.tools.cmake").install(package, configs)
    end)
package_end()
add_requires("ckdl")

target "Editor"
    set_kind("binary")
    set_languages("c++20")
	set_rundir("$(projectdir)/Editor")
    set_default(true)
    add_rules("CompilerFlags")
    set_warnings("all")

    add_includedirs (
        "Source"
    )

    add_files (
        "Source/**.cpp"
    )

    add_headerfiles (
        "Source/**.h"
    )

    add_defines(
        "REFLECT_TYPE_INFO_ENABLED"
    )

    add_extrafiles("Assets/**.shader")

    -- IMGUI
    add_sysincludedirs("Vendor/imgui")
    add_files("Vendor/imgui/*.cpp")

    add_deps("Engin5")
    add_packages("glfw", "ckdl")

    if is_plat("windows") then
        set_runtimes("MDd")
        add_ldflags("/WHOLEARCHIVE:Engin5", {force = true})
    end

    if is_plat("linux") then
        add_linkgroups("Engin5", {whole = true})
    end
