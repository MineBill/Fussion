target "Sandbox"
    set_kind("binary")
    set_languages("c++23")
    add_rules("CompilerFlags")
    add_rules("CommonFlags")

    add_options("Tracy")

    set_warnings("allextra")

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
        "FSN_MATH_USE_GLOBALLY",
        "FSN_CORE_USE_GLOBALLY"
    )

    add_deps("Fussion")

    -- if is_plat("windows") then
    --     set_runtimes("MDd")
    --     add_ldflags("/WHOLEARCHIVE:Fussion", {force = true})

    --     add_options("LivePP")
    -- end

    -- if is_plat("linux") then
    --     add_linkgroups("Fussion", {whole = true})
    -- end
