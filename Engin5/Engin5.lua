target("Engin5")
    set_kind("static")
    set_languages("c++20")
    add_ldflags("cl::/debug:fastlink")

    add_files (
        "Source/**.cpp",
        "Impl/**.cpp",

        "Vendor/tracy/public/TracyClient.cpp"
    )

    add_headerfiles (
        "Source/**.h",
        "Impl/**.h"
    )

    add_includedirs("Source/Engin5")
    add_includedirs("Source", {public = true})
    add_includedirs("Impl", {public = true})
	add_includedirs("Vendor/entt/src", {public = true})

    add_packages("glfw", "VMA")
    add_deps("magic_enum")
    add_deps("glm", {public = true})
    add_deps("Reflect")
    add_deps("AngelScript")

    add_deps("HeaderTool")
    add_rules("RunHeaderTool")

    set_pcxxheader("Source/e5pch.h")

    add_defines("TRACY_ENABLE", {public = true})
    add_sysincludedirs("Vendor/tracy/public", {public = true})

    add_defines (
        "GLM_ENABLE_EXPERIMENTAL",
        "VMA_DYNAMIC_VULKAN_FUNCTIONS",
        "REFLECT_TYPE_INFO_ENABLED",
        "VK_NO_PROTOTYPES"
    )

    add_cxxflags("cl::/EHsc")
    add_cxxflags("cl::/permissive-")

    if is_plat("windows") then
        add_defines("OS_WINDOWS", "VK_USE_PLATFORM_WIN32_KHR")
        add_links("gdi32", "user32", "shell32")
        set_runtimes("MDd")
        add_sysincludedirs(os.getenv("VULKAN_SDK") .. "/Include", {public = true})
        add_links (
            os.getenv("VULKAN_SDK") .. "/Lib/shaderc_sharedd.lib",
            os.getenv("VULKAN_SDK") .. "/Lib/spirv-cross-cored.lib",
            os.getenv("VULKAN_SDK") .. "/Lib/spirv-cross-glsld.lib"
        )

    elseif is_plat("linux") then
        add_defines("OS_LINUX", {public = true})
        add_links("shaderc_shared")

        add_packages("spirv-cross")
    elseif is_plat("macos") then
        add_defines("OS_MACOS")
    end