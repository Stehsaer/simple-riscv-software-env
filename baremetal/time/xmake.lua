target("baremetal.time")
	set_kind("static")
	set_languages("c++23", {public=true})

	add_files("src/*.cpp")
	add_includedirs("include", {public=true})