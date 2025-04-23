target("project.os-class.lab3.disk-io")

	set_kind("binary")
	add_files("disk-io.cpp")
	add_deps(
		"platform.qemu", 
		"platform.generic-boot",
		"file.interface",
		"file.driver.uart",
		"file.driver.fatfs.backend.virtio"
	)

	add_rules(
		"platform.qemu.native",
		"report-size",
		"platform.qemu.run"
	)

target("project.os-class.lab3.boot")

	set_kind("binary")
	add_files("boot.cpp")
	add_deps(
		"platform.qemu", 
		"platform.generic-boot",
		"file.interface",
		"file.driver.uart",
		"file.driver.fatfs.backend.virtio"
	)
	
	add_rules(
		"platform.qemu.native",
		"report-size",
		"platform.qemu.run"
	)

target("project.os-class.lab3.test")

	set_kind("binary")
	add_files("test.ld", "test.S")