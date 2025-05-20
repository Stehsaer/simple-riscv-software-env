target("baremetal.filesystem.driver.fatfs")

	set_kind("static")

	add_files("src/*.c", "src/*.cpp")
	add_includedirs("include", {public = true})
	add_deps("baremetal.filesystem", "baremetal.allocator", {public=true})

target_end()
