target("module.file.driver.uart")

	set_kind("static")

	add_deps("module.file.interface", "module.device", {public=true})
	add_files("src/**.cpp")
	add_includedirs("include", {public=true})