add_rules("mode.debug", "mode.release")
add_rules("plugin.vsxmake.autoupdate")
add_rules("plugin.compile_commands.autoupdate", {outputdir = ".vscode"})
set_policy("package.install_locally", true)

set_runtimes("MDd")
rule("CompilerFlags")
    on_load(function (target)
        --[[ target:add("cxxflags", "gcc::-Wno-changes-meaning")
        target:add("cxxflags", "clang::-Wno-changes-meaning") ]]
        target:add("cxxflags", "clang::-Wno-c++98-compat")
        target:add("cxxflags", "clang_cl::-ftime-trace")

        target:add("cxxflags", "clang_cl::-Wno-c++98-compat")
        target:add("cxxflags", "clang_cl::-Wno-unknown-attributes")
        target:add("cxxflags", "clang_cl::-Wno-missing-field-initializers")
        target:add("cxxflags", "clang_cl::-Wno-microsoft-include")
        target:add("cxxflags", "clang_cl::-Wno-pragma-system-header-outside-header")

        target:add("cxxflags", "cl::/EHsc")
        target:add("cxxflags", "cl::/permissive-")
        target:add("cxxflags", "cl::/wd4514 /wd4820 /wd5030 /wd5222")
    end)
rule_end()

rule("CommonFlags")
    on_load(function(target)
        target:add("defines", "USE_ASSERTIONS")
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
    add_defines("TRACY_ON_DEMAND", {public = true})
option_end();

option("TimeTrace")
    set_showmenu(true)
option_end()

includes("Vendor.lua")

includes("HeaderTool/HeaderTool.lua")
includes("Fussion/Fussion.lua")
includes("Editor/Editor.lua")
includes("Sandbox/Sandbox.lua")
