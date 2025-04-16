target("allocator")

	set_kind("static")
	set_languages("c++23", {public=true})
	
	add_files("syscall.cpp")