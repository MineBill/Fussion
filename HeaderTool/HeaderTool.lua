target("tree-sitter", function()
    set_kind "static"
    set_group "Vendor"
    set_languages("c11")

    add_files(
        "tree-sitter-0.22.3/lib/src/lib.c"
    )

    add_includedirs(
        "tree-sitter-0.22.3/lib/src",
        "tree-sitter-0.22.3/lib/include",
        {public = true}
    )
end)

target("tree-sitter-cpp", function()
    set_kind "static"
    set_group "Vendor"
    set_languages("c11")

    add_files(
        "tree-sitter-cpp/src/parser.c",
        "tree-sitter-cpp/src/scanner.c"
    )

    add_includedirs(
        "tree-sitter-cpp/bindings/c",
        {public = true}
    )

    add_includedirs(
        "tree-sitter-cpp/src"
    )
end)

target("HeaderTool", function()
    set_policy("build.across_targets_in_parallel", false)

    set_kind("binary")
    set_languages("c++23")
    set_warnings("allextra")
	set_rundir("$(projectdir)")

    add_options("Tracy")

    add_files(
        "Source/**.cpp",
        "../Fussion/Vendor/tracy/public/TracyClient.cpp"
    )
    add_sysincludedirs("../Fussion/Vendor/tracy/public", {public = true})

    add_headerfiles("Source/**.h")

    add_deps("tree-sitter", "tree-sitter-cpp")
end)

rule("HeaderTool")
    on_load(function(target)
        import("core.project.config")
        target:add("deps", "HeaderTool")
        target:add("files", config:buildir() .. "/ReflectionRegistry_Generated.cpp", {always_added = true})
    end)

    before_build(function(target)
        import("core.project.project")
        import("core.project.config")
        import("core.project.depend")
        local output_file = config:buildir() .. "/ReflectionRegistry_Generated.cpp"

        local files = {}
        for _, pattern in ipairs(target:get("headerfiles")) do
            for _, filepath in ipairs(os.files(pattern)) do
                table.append(files, filepath)
            end
        end

        local header_tool = project.target("HeaderTool")
        for _, pattern in ipairs(header_tool:get("files")) do
            for _, filepath in ipairs(os.files(pattern)) do
                table.append(files, filepath)
            end
        end

        depend.on_changed(function()
            local header_tool = project.target("HeaderTool")
            if header_tool then
                os.execv(header_tool:targetfile(), {config:buildir()})
            end
            assert(os.isfile(output_file))
        end, {files = files, changed = not os.isfile(output_file)})
    end)