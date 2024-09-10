target("FussionTests")
    set_kind("binary")
    set_languages("c++23")
    set_warnings("allextra")
    set_group("Tests")

    add_files("Source/**.cpp")
    add_headerfiles("Source/**.h")

    add_rules("CompilerFlags")
	add_rules("CommonFlags")

    add_deps("Fussion")
    add_packages("Catch2")

    add_defines("GLM_ENABLE_EXPERIMENTAL")
