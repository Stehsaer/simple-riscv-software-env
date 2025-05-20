target("project.os-class.lab3.disk-io")

	set_kind("binary")
	add_files("disk-io.cpp")
	add_deps(
		"baremetal.platform.qemu",
		"boot.stage0"
	)

	add_rules(
		"generate.qemu-flash",
		"platform.qemu.config",
		"platform.qemu.run",
		"boot.link.stage0",
		"report-size"
	)

target("project.os-class.lab3.boot")

	set_kind("binary")
	add_files("boot.cpp")
	add_deps(
		"baremetal.platform.qemu",
		"boot.stage0"
	)

	add_rules(
		"generate.qemu-flash",
		"platform.qemu.config",
		"platform.qemu.run",
		"boot.link.stage0",
		"report-size"
	)

target("project.os-class.lab3.test")

	set_kind("binary")
	add_files("test.ld", "test.S")