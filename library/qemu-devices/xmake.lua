target("qemu-devices")
	set_kind("static")

	add_deps("os-runtime")
	add_files("src/**.cpp")
	add_includedirs("include", {public=true})