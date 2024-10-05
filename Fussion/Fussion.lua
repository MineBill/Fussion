add_requires("wgpu-native")

target("Fussion")
    set_kind("static")
    set_languages("c++23")
    add_ldflags("cl::/debug:fastlink")
    set_warnings("allextra")

    add_files (
        "Source/**.cpp",
        "Source/**.c",
        "Impl/**.cpp"

        -- "Vendor/tracy/public/TracyClient.cpp"
    )
    if is_plat("windows") then
        add_files("Platform/Windows/**.cpp")
    elseif is_plat("linux") then
        add_files("Platform/Linux/**.cpp")
    end

    add_headerfiles (
        "Source/**.h",
        "Source/**.hpp",
        "Impl/**.h",
        "**.natvis"
    )

    add_rules("utils.bin2c", {extensions = {".png", ".hdr"}})
    add_files("Assets/Textures/**.png")
    add_files("Assets/Textures/**.hdr")

    add_includedirs("Source/Fussion")
    add_includedirs("Source", {public = true})
    add_includedirs("Impl", {public = true})
    add_includedirs("Vendor/entt/src", {public = true})

    add_packages("fmt", "cpptrace", {public = true})
    add_packages("glfw", "VMA", "wgpu-native", "JoltPhysics")
    add_deps("magic_enum")
    add_deps("glm", {public = true})
    add_deps("AngelScript")
    add_deps("aatc")
    add_deps("TracyClient")

    add_rules("HeaderTool")

    set_pcxxheader("Source/FussionPCH.h")

    add_defines (
        "GLM_ENABLE_EXPERIMENTAL"
    )

    add_rules("CompilerFlags")
    add_rules("CommonFlags")
    add_options("Tracy")

    if is_plat("windows") then
        add_defines("OS_WINDOWS", {public = true})
        add_links("gdi32", "user32", "shell32", "Comdlg32")
        -- set_runtimes("MDd")
        add_cxxflags("cl::/bigobj")
        add_cxxflags("cl::/utf-8", {public = true})
    elseif is_plat("linux") then
        add_defines("OS_LINUX", {public = true})
        add_links("shaderc_shared")

        add_packages("spirv-cross")
    elseif is_plat("macos") then
        add_defines("OS_MACOS")
    end
	
	-- Needed by cpptrace
	-- TODO: Figure out how to put this in the package declaration.
	if is_plat("windows") then
		add_syslinks("dbghelp", {public = true})
	elseif is_plat("macosx") then
		add_deps("libdwarf")
	elseif is_plat("linux") then
		add_syslinks("dl", "dwarf", "z", "zstd")
	elseif is_plat("mingw") then
		add_deps("libdwarf")
		add_syslinks("dbghelp")
	end

includes("Tests/FussionTests.lua")
