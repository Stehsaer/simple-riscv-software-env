target("nyan")
	set_kind("binary")
	set_languages("c99")

	add_files("*.c", "*.cpp")
	add_deps("loaded-runtime", "platform-v1")
	add_files("$(projectdir)/template/loaded-linkscript.ld")

