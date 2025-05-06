target("mutex")

	set_kind("static")
	
	add_files("mutex.cpp")
	add_includedirs("./", {public=true})