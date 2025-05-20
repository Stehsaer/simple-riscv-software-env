target("baremetal.time.platform.qemu")
	set_kind("static")

	add_deps("baremetal.time", "platform.qemu", {public=true})
	add_files("src/**.cpp")