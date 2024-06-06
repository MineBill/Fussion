target("HeaderTool")
    set_kind("binary")
    set_languages("c++23")
    set_default(false)

    add_deps("Reflect")
    add_files("Source/**.cpp")
    add_headerfiles("Source/**.h")
    set_policy("build.fence", true)
    add_defines(
        "REFLECT_TYPE_INFO_ENABLED"
    )

    if is_plat("windows") then
        set_runtimes("MDd")
    end

rule("RunHeaderTool")
    before_build(function(target, b)
        print("Running rule for target "..target:name())
        import('core.project.project')
        local tool = project.target("HeaderTool"):targetfile()
        os.execv(tool, { target:scriptdir() .. "/Source" })
    end)