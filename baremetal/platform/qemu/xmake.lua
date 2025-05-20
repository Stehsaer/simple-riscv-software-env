includes("filesystem", "time")

target("baremetal.platform.qemu")
	set_kind("phony")
	add_deps(
		"baremetal.filesystem.platform.qemu",
		"baremetal.time.platform.qemu",
		{public=true}
	)
