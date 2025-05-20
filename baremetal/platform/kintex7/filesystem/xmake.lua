target("baremetal.filesystem.platform.kintex7")
	set_kind("static")

	add_deps("platform.kintex7", "baremetal.filesystem", "baremetal.filesystem.driver.fatfs", {public=true})
	add_includedirs("include", {public=true})
	add_files("src/**.cpp")