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
        "Source/**.cpp"
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

    add_extrafiles("Assets/**.shader", "Assets/**.glsl")

    -- IMGUI
    add_sysincludedirs("Vendor/imgui")
    add_files("Vendor/imgui/*.cpp")

    add_deps("Fussion")
    add_packages("glfw")

    set_pcxxheader("Source/epch.h")

    if is_plat("windows") then
        set_runtimes("MDd")
        add_ldflags("/WHOLEARCHIVE:Fussion", {force = true})

        add_options("LivePP")
    end

    if is_plat("linux") then
        add_linkgroups("Fussion", {whole = true})
    end
