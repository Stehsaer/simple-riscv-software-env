target("project.os-class.lab2.screen")

	set_kind("binary")
	add_files("screen.cpp")
	add_deps(
		"module.platform.qemu", 
		"module.start.boot",
		"module.file.interface",
		"module.file.driver.uart"
	)

	add_rules(
		"module.platform.qemu.native",
		"generate.map"
	)

	on_run(run_target_qemu)

target_end()

target("project.os-class.lab2.helloworld")

	set_kind("binary")
	add_files("helloworld.cpp")
	add_deps(
		"module.platform.qemu", 
		"module.start.boot",
		"module.file.interface",
		"module.file.driver.uart"
	)

	add_rules(
		"module.platform.qemu.native",
		"generate.map"
	)

	on_run(run_target_qemu)

target_end()
