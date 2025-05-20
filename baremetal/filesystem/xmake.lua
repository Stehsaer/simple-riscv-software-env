target("baremetal.filesystem")

	set_kind("static")

	add_deps("baremetal.allocator", "baremetal.mutex", {public=true})

	add_files("src/**.cpp")
	add_includedirs("include", {public=true})

target_end()

includes("driver")