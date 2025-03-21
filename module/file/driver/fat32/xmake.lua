target("module.file.driver.fatfs")

	set_kind("static")

	add_files("src/*.c", "src/*.cpp")
	add_includedirs("include", {public = true})
	add_deps("module.file.interface", "module.allocator", {public=true})

target_end()

includes("backend")