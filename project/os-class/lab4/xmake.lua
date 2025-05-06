target("project.os-class.lab4.main")

	set_kind("binary")
	add_files("main.cpp")
	add_files("*.S")
	add_deps(
		"platform.qemu", 
		"platform.generic-boot",
		"file.interface",
		"file.driver.uart"
	)

	add_rules(
		"platform.qemu.native",
		"report-size",
		"platform.qemu.run"
	)