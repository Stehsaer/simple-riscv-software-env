target("module.file.interface")

	set_kind("static")

	add_deps("module.allocator", {public=true})

	add_files("src/**.cpp")
	add_includedirs("include", {public=true})

target_end()

includes("driver")