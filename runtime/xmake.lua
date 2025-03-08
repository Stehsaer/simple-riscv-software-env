target("raw-runtime")

	set_kind("static")
	add_files("raw-start.s", "fast-memset.s")

target_end()

target("loaded-runtime")

	set_kind("static")
	add_files("loaded-start.s")

target_end()