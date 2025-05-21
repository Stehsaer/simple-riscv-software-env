target("project.os-class.lab4.main")

	set_kind("binary")
	add_files("main.cpp")
	add_files("*.S")
	add_deps(
		"baremetal.platform.qemu",
		"boot.stage0"
	)

	add_rules(
		"generate.qemu-flash",
		"platform.qemu.config",
		"platform.qemu.run",
		"link.stage0",
		"report-size"
	)