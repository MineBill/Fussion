target "Editor"
    set_kind("binary")
    set_languages("c++20")
	set_rundir("$(projectdir)/Editor")

    add_includedirs (
        "Source"
    )

    add_files (
        "Source/**.cpp"
    )

    add_headerfiles (
        "Source/**.h"
    )

    add_extrafiles("Assets/**.shader")

    -- IMGUI
    add_sysincludedirs("Vendor/imgui")
    add_files("Vendor/imgui/*.cpp")

    add_deps("Engin5")
    add_packages("glfw")

    if is_plat("windows") then
        set_runtimes("MDd")
    end
