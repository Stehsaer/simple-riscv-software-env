target("project.os-class.lab7")
	set_kind("binary")

	add_files("main.cpp", "program.S")

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

	add_cxflags("-fPIC")
	add_asflags("-fPIC")
