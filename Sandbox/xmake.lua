target("Sandbox")
    set_kind("binary")
    set_languages("c++20")
    add_files("Source/**.cpp")
    add_extrafiles("Assets/**.shader")
    -- add_headerfiles("Source/**.h")
    
    add_deps("Engin5")

    if is_plat("windows") then
        set_runtimes("MDd")
    end
