
target("test-ground")
	set_kind("binary")
	
	add_deps("loaded-runtime", "os-runtime", "platform-v1", "elfio")
	add_files("main.cpp", "$(projectdir)/template/loaded-linkscript.ld")
