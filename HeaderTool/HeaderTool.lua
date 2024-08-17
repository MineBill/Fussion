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
    set_kind("binary")
    set_languages("c++23")
    set_warnings("allextra")
	set_rundir("$(projectdir)")

    add_files(
        "Source/**.cpp"
    )

    add_headerfiles(
        "Source/**.h"
    )

    add_deps("tree-sitter", "tree-sitter-cpp")
end)