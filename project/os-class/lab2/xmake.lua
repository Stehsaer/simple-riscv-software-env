target("project.os-class.lab2.screen")

	set_kind("binary")
	add_files("screen.cpp")
	add_deps(
		"baremetal.platform.qemu"
	)

	add_rules(
		"generate.qemu-flash",
		"platform.qemu.config",
		"platform.qemu.run",
		"link.stage0",
		"report-size"
	)

target_end()

target("project.os-class.lab2.helloworld")

	set_kind("binary")
	add_files("helloworld.cpp")
	add_deps(
		"baremetal.platform.qemu"
	)

	add_rules(
		"generate.qemu-flash",
		"platform.qemu.config",
		"platform.qemu.run",
		"link.stage0",
		"report-size"
	)


target_end()
