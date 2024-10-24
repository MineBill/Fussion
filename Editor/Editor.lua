includes("Vendor/xmake.lua")

target "Editor"
    set_kind("binary")
    set_languages("c++23")
	set_rundir("$(projectdir)/Editor")
    set_default(true)
    add_rules("CompilerFlags")
    add_rules("CommonFlags")

    add_options("Tracy")

    set_warnings("allextra")

    add_rules("utils.bin2c", {extensions = {".png"}})
    add_files("Assets/Icons/logo_*.png")

    add_includedirs (
        "Source"
    )

    add_files (
        "Source/**.cpp",
        "Source/**.c"
    )

    add_headerfiles (
        "Source/**.h"
    )

    add_defines(
        "GLM_ENABLE_EXPERIMENTAL",
        "USE_EDITOR",
        "FSN_MATH_USE_GLOBALLY",
        "FSN_CORE_USE_GLOBALLY"
    )

    add_extrafiles("Assets/**.wgsl", "Assets/**.slang")

    -- IMGUI
    add_sysincludedirs("Vendor/imgui")
    add_files("Vendor/imgui/*.cpp")

    add_deps("Fussion", "argparse")
    -- We need an explicit link to glfw here to be able to build the glfw imgui backend.
    add_packages("glfw", "wgpu-native-custom", "meshoptimizer")

    set_pcxxheader("Source/EditorPCH.h")

    if is_plat("windows") then
        -- set_runtimes("MDd")
        add_ldflags("/WHOLEARCHIVE:Fussion", {force = true})

        add_options("LivePP")
    end

    if is_plat("linux") then
        add_linkgroups("Fussion", {whole = true})
    end
