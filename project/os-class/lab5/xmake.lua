target("project.os-class.lab5")

	set_kind("binary")
	add_files("main.cpp", "linkscript.ld")
	add_files("*.S")
	add_deps(
		"platform.qemu", 
		"platform.generic-boot",
		"file.interface",
		"file.driver.uart"
	)

	add_rules(
		"report-size",
		"platform.qemu.run",
		"generate.qemu-flash"
	)

target("project.os-class.lab5.va")

	set_kind("binary")
	add_files("va.cpp")

	add_deps(
		"platform.qemu", 
		"platform.generic-boot",
		"file.interface",
		"file.driver.uart"
	)

	add_rules(
		"report-size",
		"platform.qemu.run",
		"generate.qemu-flash",
		"platform.qemu.full-native"
	)
