target("platform-v1")
	set_languages("c++23", {public=true})
	set_kind("static")

	add_files("src/*.cpp")
	add_includedirs("include", {public=true})
	add_deps("device&driver", "os-runtime")