target("elfio")
	set_kind("headeronly")
	set_languages("c++23", {public=true})

	add_includedirs("include", {public=true})
	add_headerfiles("include/**.hpp", {public=true})