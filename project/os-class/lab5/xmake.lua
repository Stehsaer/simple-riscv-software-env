target("project.os-class.lab5")

	set_kind("binary")
	set_enabled(has_config("rvext_a"))

	add_files("main.cpp", "linkscript.ld")
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

target_end()

target("project.os-class.lab5.va")

	set_kind("binary")
	add_files("va.cpp")

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
target_end()
