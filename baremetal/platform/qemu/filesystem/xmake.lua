target("baremetal.filesystem.platform.qemu")
	set_kind("static")

	add_deps("platform.qemu", "baremetal.filesystem", "baremetal.filesystem.driver.fatfs", {public=true})
	add_includedirs("include", {public=true})
	add_files("src/**.cpp")