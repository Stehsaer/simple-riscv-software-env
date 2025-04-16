target("platform.kintex7")
	set_kind("static")
	set_languages("c++23", "c23", {public=true})

	add_deps("device", {public=true})
	add_includedirs("include", {public=true})
	add_files("src/**.cpp")

target_end()

