target("Sandbox")
    set_kind("binary")
    set_languages("c++23")
    add_files("Source/**.cpp")
    add_extrafiles("Assets/**.shader")
    set_rundir("$(projectdir)/Sandbox")
    set_default(false)

    add_deps("Fussion")

    if is_plat("windows") then
        set_runtimes("MDd")
    end
