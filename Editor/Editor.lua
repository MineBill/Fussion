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
        "REFLECT_TYPE_INFO_ENABLED",
        "USE_EDITOR"
    )

    add_extrafiles("Assets/**.shader")

    -- IMGUI
    add_sysincludedirs("Vendor/imgui")
    add_files("Vendor/imgui/*.cpp")

    add_deps("Engin5", "kdl")
    add_packages("glfw")

    add_deps("HeaderTool")
    add_rules("RunHeaderTool")

    if is_plat("windows") then
        set_runtimes("MDd")
        add_ldflags("/WHOLEARCHIVE:Engin5", {force = true})
    end

    if is_plat("linux") then
        add_linkgroups("Engin5", {whole = true})
    end