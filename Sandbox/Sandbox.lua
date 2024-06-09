target("Sandbox")
    set_kind("binary")
    set_languages("c++23")
    add_files("Source/**.cpp")
    add_extrafiles("Assets/**.shader")
    set_rundir("$(projectdir)/Sandbox")
    
    add_deps("Engin5")

    if is_plat("windows") then
        set_runtimes("MDd")
    end
