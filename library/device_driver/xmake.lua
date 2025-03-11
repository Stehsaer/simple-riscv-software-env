target("device&driver-readonly")
	set_kind("static")
	set_languages("c++23", "c23", {public=true})

	add_files("src/**.cpp", "src/**.c")
	add_includedirs("include", {public=true})
	add_defines("FS_CFG_READONLY", {public=true})
	add_deps("os-runtime", {public=true})

target("device&driver")
	set_kind("static")
	set_languages("c++23", "c23", {public=true})

	add_files("src/**.cpp", "src/**.c")
	add_includedirs("include", {public=true})
	add_deps("os-runtime", {public=true})