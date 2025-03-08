add_requires("stb", "libjpeg")

target("image-test")
	set_kind("binary")

	add_deps("loaded-runtime", "platform-v1")
	add_files("main.cpp", "$(projectdir)/template/loaded-linkscript.ld")
	add_packages("stb", "libjpeg")
