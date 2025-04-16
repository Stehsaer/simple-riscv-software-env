target("file.driver.fatfs.backend.virtio")
	set_kind("static")

	add_deps("file.driver.fatfs", "device", {public=true})
	add_includedirs("include", {public=true})
	add_files("src/**.cpp")