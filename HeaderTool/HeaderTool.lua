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

    add_rules("CompilerFlags")

    if is_plat("windows") then
        set_runtimes("MDd")
        add_defines("OS_WINDOWS")
    elseif is_plat("linux") then
        add_defines("OS_LINUX")
    elseif is_plat("macos") then
        add_defines("OS_MACOS")
    end

rule("RunHeaderTool")
    before_build(function(target, b)
        print("Running rule for target "..target:name())
        import('core.project.project')
        local tool = project.target("HeaderTool"):targetfile()
        os.execv(tool, { target:scriptdir() .. "/Source" })
    end)