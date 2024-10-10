package("glfw")
    add_deps("cmake")
    set_sourcedir(path.join(os.scriptdir(), "Vendor/glfw"))
    on_install(function (package)
        local configs = {}
        table.insert(configs, "-DCMAKE_BUILD_TYPE=" .. (package:debug() and "Debug" or "Release"))
        table.insert(configs, "-DBUILD_SHARED_LIBS=" .. (package:config("shared") and "ON" or "OFF"))
        table.insert(configs, "-DGLFW_BUILD_TESTS=OFF")
        table.insert(configs, "-DGLFW_BUILD_EXAMPLES=OFF")
        table.insert(configs, "-DGLFW_BUILD_DOCS=OFF")
        if is_plat("linux") then
            -- For linux, default to X11, Wayland(Waycrap) is very broken with imgui windows.
            -- Also, RenderDoc doesn't like Wayland.
            table.insert(configs, "-DGLFW_BUILD_X11=ON")
            table.insert(configs, "-DGLFW_BUILD_WAYLAND=OFF")
        end
        import("package.tools.cmake").install(package, configs)
    end)
package_end()
add_requires("glfw")

package("fmt")
    add_deps("cmake")
    set_sourcedir(path.join(os.scriptdir(), "Vendor/fmt-11.0.2"))
    on_install(function (package)
        local configs = {}
        table.insert(configs, "-DCMAKE_BUILD_TYPE=" .. (package:debug() and "Debug" or "Release"))
        table.insert(configs, "-DBUILD_SHARED_LIBS=" .. (package:config("shared") and "ON" or "OFF"))
        local defines = {
            "-DFMT_DOC=OFF",
            "-DFMT_TEST=OFF",
            "-DFMT_FUZZ=OFF",
            "-DFMT_CUDA_TEST=OFF"
        }
        for k, v in pairs(defines) do
            table.insert(configs, v)
        end
        import("package.tools.cmake").install(package, configs)
    end)
package_end()
add_requires("fmt")

package("cpptrace")
    add_deps("cmake")
    set_sourcedir(path.join(os.scriptdir(), "Vendor/cpptrace-0.6.3"))
    on_install(function (package)
        local configs = {}
        table.insert(configs, "-DCMAKE_BUILD_TYPE=" .. (package:debug() and "Debug" or "Release"))
        table.insert(configs, "-DBUILD_SHARED_LIBS=" .. (package:config("shared") and "ON" or "OFF"))
		package:add("links", "cpptrace")
        if not package:config("shared") then
            package:add("defines", "CPPTRACE_STATIC_DEFINE")
        end
        import("package.tools.cmake").install(package, configs)
    end)
package_end()
add_requires("cpptrace")

package("tinygltf")
    add_deps("cmake")
    set_sourcedir(path.join(os.scriptdir(), "Vendor/tinygltf-2.8.23"))
    on_install(function (package)
        local configs = {}
        table.insert(configs, "-DCMAKE_BUILD_TYPE=" .. (package:debug() and "Debug" or "Release"))
        table.insert(configs, "-DBUILD_SHARED_LIBS=" .. (package:config("shared") and "ON" or "OFF"))
        import("package.tools.cmake").install(package, configs)
    end)
package_end()
add_requires("tinygltf")

package("Catch2")
    add_deps("cmake")
    set_sourcedir(path.join(os.scriptdir(), "Vendor/Catch2-3.6.0"))
    on_install(function (package)
        local configs = {}
		table.insert(configs, "-DCMAKE_CXX_STANDARD=20")
        table.insert(configs, "-DCMAKE_BUILD_TYPE=" .. (package:debug() and "Debug" or "Release"))
        table.insert(configs, "-DBUILD_SHARED_LIBS=" .. (package:config("shared") and "ON" or "OFF"))
        import("package.tools.cmake").install(package, configs)
    end)
package_end()
add_requires("Catch2")

package("JoltPhysics")
    add_deps("cmake")
    set_sourcedir(path.join(os.scriptdir(), "Fussion/Vendor/JoltPhysics/Build"))
    on_install(function (package)
        local configs = {}
        table.insert(configs, "-DTARGET_UNIT_TESTS=OFF")
		table.insert(configs, "-DTARGET_HELLO_WORLD=OFF")
		table.insert(configs, "-DTARGET_PERFORMANCE_TEST=OFF")
		table.insert(configs, "-DTARGET_SAMPLES=OFF")
		table.insert(configs, "-DTARGET_VIEWER=OFF")
		table.insert(configs, "-DUSE_STATIC_MSVC_RUNTIME_LIBRARY=OFF")
		table.insert(configs, "-DINTERPROCEDURAL_OPTIMIZATION=OFF")
		table.insert(configs, "-DUSE_ASSERTS=ON")
		table.insert(configs, "-DENABLE_ALL_WARNINGS=OFF")

        table.insert(configs, "-DCMAKE_BUILD_TYPE=" .. (package:debug() and "Debug" or "Release"))
        table.insert(configs, "-DBUILD_SHARED_LIBS=" .. (package:config("shared") and "ON" or "OFF"))
        import("package.tools.cmake").install(package, configs)
    end)
package_end()
add_requires("JoltPhysics")

target("glm", function()
    set_kind "headeronly"
    set_group "Vendor"
    set_languages("c++23")

    add_includedirs("Vendor/glm", {public = true})
end)

target("AngelScript", function()
    set_kind "static"
    set_languages "c++20"
    set_version "2.36.1"
    set_group "Vendor"

    add_files(
        "Fussion/Vendor/angelscript/sdk/angelscript/source/**.cpp",
        "Fussion/Vendor/angelscript/sdk/add_on/**.cpp|autowrapper/generator/generateheader.cpp"
    )
    add_headerfiles(
        "Fussion/Vendor/angelscript/sdk/angelscript/source/**.h",
        "Fussion/Vendor/angelscript/sdk/add_on/**.h"
    )
    add_sysincludedirs("Fussion/Vendor/angelscript/sdk/angelscript/include", {public = true})
    add_sysincludedirs("Fussion/Vendor/angelscript/sdk/add_on/", {public = true})

    if is_plat("windows") then
        add_files "Fussion/Vendor/angelscript/sdk/angelscript/source/as_callfunc_x64_msvc_asm.asm"
    end
end)

target("aatc", function()
    set_kind "static"
    set_languages "c++20"
    set_version "0.0.1"
    set_group "Vendor"

    add_deps("AngelScript")
    add_files("Vendor/aatc/source/**.cpp")
    add_headerfiles("Vendor/aatc/source/**.hpp")
    add_sysincludedirs("Vendor/aatc/source/", {public = true})
end)

target("TracyClient", function()
    set_kind("static")
    set_languages "c++20"
    set_group("Vendor")
    add_files(
        "Fussion/Vendor/tracy/public/TracyClient.cpp"
    )

    add_headerfiles(
        "Fussion/Vendor/tracy/public/common/*.hpp",
        "Fussion/Vendor/tracy/public/tracy/*.hpp"
    )

    add_sysincludedirs("Fussion/Vendor/tracy/public", {public = true})

    if is_plat("windows") then
        -- set_runtimes "MDd"
    end

    add_defines("TRACY_ENABLE", {public = true})
    add_defines("TRACY_ON_DEMAND", {public = true})
end)

target("magic_enum", function()
    set_kind "headeronly"
    set_group "Vendor"
    set_languages("c++23")

    add_sysincludedirs("Vendor/magic_enum/include", {public = true})
end)

target("argparse", function()
    set_kind "headeronly"
    set_group "Vendor"
    set_languages("c++23")

    add_sysincludedirs("Vendor/argparse/include", {public = true})
end)

package("wgpu-native")
    set_homepage("https://github.com/gfx-rs/wgpu-native")
    set_description("Native WebGPU implementation based on wgpu-core")
    set_license("Apache-2.0")

    if is_plat("windows") and is_arch("x64") then
        add_urls("https://github.com/gfx-rs/wgpu-native/releases/download/$(version)/wgpu-windows-x86_64-msvc-release.zip", {version = function(version) return version:gsub("%+", ".") end})
        add_versions("v22.1.0+5", "81b1c6d83c1a9b8d507d6c5862c71d45b877ff952488daf7bc53edc3817ae3b8")
    elseif is_plat("linux") and is_arch("x86_64") then
        add_urls("https://github.com/gfx-rs/wgpu-native/releases/download/$(version)/wgpu-linux-x86_64-release.zip", {version = function(version) return version:gsub("%+", ".") end})
        add_versions("v22.1.0+5", "851984418f237aae593cda2a6f44a03afc1cc6e444662222ed2f9f6bc4c776fc")
    end

    if is_plat("windows") then
        add_configs("vs_runtime", {description = "Set vs compiler runtime.", default = "MD", readonly = true})
    end

    add_includedirs("include", "include/webgpu")

    on_load("windows", function (package)
        if not package:config("shared") then
            package:add("syslinks", "Advapi32", "bcrypt", "d3dcompiler", "NtDll", "User32", "Userenv", "WS2_32", "Gdi32", "Opengl32", "OleAut32", "Ole32")
        end
    end)

    on_load("linux", function (package)
        if not package:config("shared") then
            package:add("syslinks", "dl", "pthread")
        end
    end)

    on_install("windows|x64", "windows|x86", "linux|arm64-v8a", "linux|x86_64", function (package)
        os.cp("include/**.h", package:installdir("include", "webgpu"))
        if package:is_plat("windows") then
            if package:config("shared") then
                os.cp("lib/wgpu_native.dll", package:installdir("bin"))
                os.cp("lib/wgpu_native.pdb", package:installdir("bin"))
                os.cp("lib/wgpu_native.dll.lib", package:installdir("lib"))
            else
                os.cp("lib/wgpu_native.lib", package:installdir("lib"))
            end
        elseif package:is_plat("linux") then
            if package:config("shared") then
                os.cp("lib/libwgpu_native.so", package:installdir("bin"))
            else
                os.cp("lib/libwgpu_native.a", package:installdir("lib"))
            end
        end
    end)

    on_test(function (package)
        assert(package:has_cfuncs("wgpuCreateInstance", {includes = "wgpu.h"}))
    end)
package_end()

package("wgpu-native-custom")
    set_homepage("https://github.com/MineBill/wgpu-native")
    set_description("Native WebGPU implementation based on wgpu-core")
    set_license("Apache-2.0")

    if is_plat("windows") and is_arch("x64") then
        add_urls("https://github.com/MineBill/wgpu-native/releases/download/$(version)/wgpu-windows-x86_64-msvc-release.zip", {version = function(version) return version:gsub("%+", ".") end})
        add_versions("v22.1.0+5-custom", "afc9c5e9d4fe1625e2e1c906b8b7ef950b17605e1ada25d8d834adeb928eeb53")
    elseif is_plat("linux") and is_arch("x86_64") then
        add_urls("https://github.com/MineBill/wgpu-native/releases/download/$(version)/wgpu-linux-x86_64-release.zip", {version = function(version) return version:gsub("%+", ".") end})
        add_versions("v22.1.0+5-custom", "d28938ff05d641e61872953010e9a960cac5c60fa0fbfe70a9368399c7e423b7")
    end

    if is_plat("windows") then
        add_configs("vs_runtime", {description = "Set vs compiler runtime.", default = "MD", readonly = true})
    end

    add_includedirs("include", "include/webgpu")

    on_load("windows", function (package)
        if not package:config("shared") then
            package:add("syslinks", "Advapi32", "bcrypt", "d3dcompiler", "NtDll", "User32", "Userenv", "WS2_32", "Gdi32", "Opengl32", "OleAut32", "Ole32")
        end
    end)

    on_load("linux", function (package)
        if not package:config("shared") then
            package:add("syslinks", "dl", "pthread")
        end
    end)

    on_install("windows|x64", "windows|x86", "linux|arm64-v8a", "linux|x86_64", function (package)
        os.cp("include/**.h", package:installdir("include", "webgpu"))
        if package:is_plat("windows") then
            if package:config("shared") then
                os.cp("lib/wgpu_native.dll", package:installdir("bin"))
                os.cp("lib/wgpu_native.pdb", package:installdir("bin"))
                os.cp("lib/wgpu_native.dll.lib", package:installdir("lib"))
            else
                os.cp("lib/wgpu_native.lib", package:installdir("lib"))
            end
        elseif package:is_plat("linux") then
            if package:config("shared") then
                os.cp("lib/libwgpu_native.so", package:installdir("bin"))
            else
                os.cp("lib/libwgpu_native.a", package:installdir("lib"))
            end
        end
    end)

    on_test(function (package)
        assert(package:has_cfuncs("wgpuCreateInstance", {includes = "wgpu.h"}))
    end)
package_end()

package("slang")
    set_homepage("https://github.com/shader-slang/slang")
    set_description("Making it easier to work with shaders")
    set_license("MIT")

    add_urls("https://github.com/shader-slang/slang.git")

    add_versions("v2024.13",   "67246517140cd509b6ebb36c8042783a0afb3dc9")
    add_versions("v2024.1.18", "efdbb954c57b89362e390f955d45f90e59d66878")
    add_versions("v2024.1.17", "62b7219e715bd4c0f984bcd98c9767fb6422c78f")

    add_configs("shared", { description = "Build shared library", default = true, type = "boolean", readonly = true })
    add_configs("embed_stdlib_source", { description = "Embed stdlib source in the binary", default = true, type = "boolean" })
    add_configs("embed_stdlib", { description = "Build slang with an embedded version of the stdlib", default = false, type = "boolean" })
    add_configs("full_ir_validation", { description = "Enable full IR validation (SLOW!)", default = false, type = "boolean" })
    add_configs("gfx", { description = "Enable gfx targets", default = false, type = "boolean" })
    add_configs("slangd", { description = "Enable language server target", default = false, type = "boolean" })
    add_configs("slangc", { description = "Enable standalone compiler target", default = false, type = "boolean" })
    add_configs("slangrt", { description = "Enable runtime target", default = false, type = "boolean" })
    add_configs("slang_glslang", { description = "Enable glslang dependency and slang-glslang wrapper target", default = false, type = "boolean" })
    add_configs("slang_llvm_flavor", { description = "How to get or build slang-llvm (available options: FETCH_BINARY, USE_SYSTEM_LLVM, DISABLE)", default = "DISABLE", type = "string" })

    add_deps("cmake")

    on_install("windows|x64", "macosx", "linux|x86_64", function (package)
        local configs = {"-DSLANG_ENABLE_TESTS=OFF", "-DSLANG_ENABLE_EXAMPLES=OFF"}
        table.insert(configs, "-DCMAKE_BUILD_TYPE=" .. (package:is_debug() and "Debug" or "Release"))
        table.insert(configs, "-DSLANG_LIB_TYPE=" .. (package:config("shared") and "SHARED" or "STATIC"))
        table.insert(configs, "-DSLANG_EMBED_STDLIB_SOURCE=" .. (package:config("embed_stdlib_source") and "ON" or "OFF"))
        table.insert(configs, "-DSLANG_EMBED_STDLIB=" .. (package:config("embed_stdlib") and "ON" or "OFF"))
        table.insert(configs, "-DSLANG_ENABLE_FULL_IR_VALIDATION=" .. (package:config("full_ir_validation") and "ON" or "OFF"))
        table.insert(configs, "-DSLANG_ENABLE_ASAN=" .. (package:config("asan") and "ON" or "OFF"))
        table.insert(configs, "-DSLANG_ENABLE_GFX=" .. (package:config("gfx") and "ON" or "OFF"))
        table.insert(configs, "-DSLANG_ENABLE_SLANGD=" .. (package:config("slangd") and "ON" or "OFF"))
        table.insert(configs, "-DSLANG_ENABLE_SLANGC=" .. (package:config("slangc") and "ON" or "OFF"))
        table.insert(configs, "-DSLANG_ENABLE_SLANGRT=" .. (package:config("slangrt") and "ON" or "OFF"))
        table.insert(configs, "-DSLANG_ENABLE_SLANG_GLSLANG=" .. (package:config("slang_glslang") and "ON" or "OFF"))
        table.insert(configs, "-DSLANG_SLANG_LLVM_FLAVOR=" .. package:config("slang_llvm_flavor"))

        import("package.tools.cmake").install(package, configs)
        package:addenv("PATH", "bin")
    end)

    on_test(function (package)
        assert(package:check_cxxsnippets({ test = [[
            #include <slang-com-ptr.h>
            #include <slang.h>

            void test() {
                Slang::ComPtr<slang::IGlobalSession> global_session;
                slang::createGlobalSession(global_session.writeRef());
            }
        ]] }, {configs = {languages = "c++17"}}))
    end)
package_end()