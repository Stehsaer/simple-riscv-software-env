target("bootloader")
	set_kind("binary")

	add_deps("raw-runtime", "platform-v1-readonly", "utility")
	add_files("main.cpp", "$(projectdir)/template/raw-linkscript.ld")

	add_links("c_nano")
	add_cxxflags("-fno-exceptions", "-fno-rtti", "-fno-unroll-loops", "-fno-inline-functions")
	
	after_build(generate_coe)