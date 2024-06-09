add_rules("mode.debug", "mode.release")
add_rules("plugin.vsxmake.autoupdate")

set_warnings("allextra")

includes("Vendor.lua")
includes("HeaderTool/HeaderTool.lua")
includes("Engin5/Engin5.lua")
includes("Editor/Editor.lua")
includes("Sandbox/Sandbox.lua")