rule("autogen")
    set_extensions(".h")
    before_buildcmd_file(function (target, batchcmds, sourcefile, opt)
        -- local sourcefile_cx = path.join(target:autogendir(), "rules", "autogen", path.basename(sourcefile) .. ".cpp")
        -- local objectfile = target:objectfile(sourcefile_cx)
        -- table.insert(target:objectfiles(), objectfile)

        if string.find(sourcefile, "_reflect_generated") then
            local source = sourcefile:gsub("_reflect_generated", ""):gsub("Generated\\", "")
            batchcmds:show_progress(opt.progress, "Running reflection tool on header %s", source)
            batchcmds:set_depmtime(os.mtime(source))
        end

        -- batchcmds:show_progress(opt.progress, "${color.build.object}compiling.autogen %s", sourcefile)
        -- batchcmds:mkdir(path.directory(sourcefile_cx))
        -- batchcmds:vrunv(target:dep("autogen"):targetfile(), {sourcefile, sourcefile_cx})
        -- batchcmds:compile(sourcefile_cx, objectfile)

        -- batchcmds:add_depfiles(sourcefile, target:dep("autogen"):targetfile())
        -- batchcmds:set_depmtime(os.mtime(objectfile))
        -- batchcmds:set_depcache(target:dependfile(objectfile))
    end)

target("Engin5")
    set_kind("static")
    set_languages("c++20")
    add_ldflags("cl::/debug:fastlink")

    add_files (
        "Source/**.cpp",
        "Impl/**.cpp",

        "Vendor/tracy/public/TracyClient.cpp"
    )

    -- add_files (
    --     "Source/**.h",
    --     "Impl/**.h"
    -- )

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
    add_deps("HeaderTool")

    add_rules("RunHeaderTool")
    -- add_rules("autogen")

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