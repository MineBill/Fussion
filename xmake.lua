add_rules("mode.debug", "mode.release")
add_rules("plugin.vsxmake.autoupdate")
add_rules("plugin.compile_commands.autoupdate", {outputdir = ".vscode"})
set_policy("package.install_locally", true)

set_runtimes("MDd")

rule("CompilerFlags")
    on_load(function (target)
        target:add("cxxflags", "gcc::-Wno-changes-meaning")
        target:add("cxxflags", "clang::-Wno-changes-meaning")
        target:add("cxxflags", "clang::-Wno-c++98-compat")

        target:add("cxxflags", "cl::/EHsc")
        target:add("cxxflags", "cl::/permissive-")
        target:add("cxxflags", "cl::/wd4514 /wd4820")
    end)
rule_end()

option("LivePP")
    set_showmenu(true)
    add_defines("FSN_LIVEPP_ENABLED")
    add_ldflags("/FUNCTIONPADMIN", "/OPT:NOREF", "/OPT:NOICF", "/Z7")
option_end()

option("Tracy")
    set_showmenu(true)
    add_defines("TRACY_ENABLE", {public = true})
option_end();

includes("Vendor.lua")
includes("Fussion/Fussion.lua")
includes("Editor/Editor.lua")
