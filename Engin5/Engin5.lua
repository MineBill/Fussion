project "Engin5"
    kind "StaticLib"
    language "C++"
    cppdialect "C++20"
    targetdir "Binaries/%{cfg.buildcfg}"
    staticruntime "off"

    pchheader "e5pch.h"
    pchsource "Source/e5pch.cpp"

    files {
        "Source/**.h",
        "Source/**.cpp",
        "Impl/**.h",
        "Impl/**.cpp",
        "Impl/**.c",

        "../Vendor/glm/glm/**.hpp",
        "../Vendor/glm/glm/**.inl",
    }

    includedirs
    {
        "Source",

        "%{IncludeDir.glfw}",
        "%{IncludeDir.glm}",
        "%{IncludeDir.VulkanSDK}",
        "%{IncludeDir.magic_enum}",
    }

    links
    {
        "GLFW",
    }

    defines
    {
        "GLM_ENABLE_EXPERIMENTAL",
        "VMA_DYNAMIC_VULKAN_FUNCTIONS",
    }

    warnings "Extra"

    targetdir ("../Binaries/" .. OutputDir .. "/%{prj.name}")
    objdir ("../Binaries/Intermediates/" .. OutputDir .. "/%{prj.name}")

    filter "system:windows"
        systemversion "latest"
        defines { "OS_WINDOWS", "VK_USE_PLATFORM_WIN32_KHR" }

    filter "system:linux"
       defines { "OS_LINUX" }

    filter "configurations:Debug"
        defines { "DEBUG" }
        runtime "Debug"
        symbols "On"

    filter "configurations:Release"
        defines { "RELEASE" }
        runtime "Release"
        optimize "On"
        symbols "On"

    filter "configurations:Dist"
        defines { "DIST" }
        runtime "Release"
        optimize "On"
        symbols "Off"
