includes("filesystem", "time")

target("baremetal.platform.kintex7")
	set_kind("phony")
	add_deps(
		"baremetal.filesystem.platform.kintex7",
		"baremetal.time.platform.kintex7",
		{public=true}
	)
