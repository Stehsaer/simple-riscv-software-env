target("project.os-class.lab2.screen")

	set_kind("binary")
	add_files("screen.cpp")
	add_deps(
		"module.platform.qemu", 
		"module.platform.generic-boot",
		"module.file.interface",
		"module.file.driver.uart"
	)

	add_rules(
		"module.platform.qemu.native",
		"report-size"
	)

target_end()

target("project.os-class.lab2.helloworld")

	set_kind("binary")
	add_files("helloworld.cpp")
	add_deps(
		"module.platform.qemu", 
		"module.platform.generic-boot",
		"module.file.interface",
		"module.file.driver.uart"
	)

	add_rules(
		"module.platform.qemu.native",
		"report-size"
	)

target_end()
