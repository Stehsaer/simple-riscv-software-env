target("bootloader")
	set_kind("binary")

	add_deps("raw-runtime", "platform-v1", "utility")
	add_files("main.cpp", "$(projectdir)/template/raw-linkscript.ld")

	add_links("c_nano")
	add_cxxflags("-fno-exceptions", "-fno-rtti")
	
	after_build(generate_coe)