target("module.file.driver.fatfs.backend.virtio")
	set_kind("static")

	add_deps("module.file.driver.fatfs", "module.device", {public=true})
	add_includedirs("include", {public=true})
	add_files("src/**.cpp")