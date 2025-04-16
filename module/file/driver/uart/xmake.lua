target("file.driver.uart")

	set_kind("static")

	add_deps("file.interface", "device", {public=true})
	add_files("src/**.cpp")
	add_includedirs("include", {public=true})