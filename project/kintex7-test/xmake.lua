target("project.kintex7-test")

	set_kind("binary")
	add_files("main.cpp")
	add_deps(
		"module.platform.kintex7", 
		"module.start.boot",
		"module.file.interface",
		"module.file.driver.uart",
		"module.file.driver.fatfs",
		"module.file.driver.fatfs.backend.sd"
	)

	add_rules(
		"module.platform.kintex7.native",
		"generate.map"
	)

	add_syslinks("c_nano")