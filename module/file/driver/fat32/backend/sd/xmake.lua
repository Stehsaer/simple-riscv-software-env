target("module.file.driver.fatfs.backend.sd")
	set_kind("static")

	add_deps("module.file.driver.fatfs", {public=true})
	add_includedirs("include", {public=true})
	add_files("src/**.cpp")