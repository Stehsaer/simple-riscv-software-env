target("project.os-class.lab5")

	set_kind("binary")
	-- set_enabled(has_isa("a"))

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

target_end()

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
target_end()
