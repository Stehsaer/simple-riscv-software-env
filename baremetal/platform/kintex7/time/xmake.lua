target("baremetal.time.platform.kintex7")
	set_kind("static")

	add_deps("baremetal.time", "platform.kintex7", {public=true})
	add_files("src/**.cpp")