target("project.os-class.lab6.scheduler")
	set_kind("static")
	set_languages("c++23", {public=true})

	set_enabled(has_config("rvext_a"))

	add_files("scheduler/src/*.S", "scheduler/src/*.cpp")
	add_includedirs("scheduler/include", {public=true})
	add_deps(
		"baremetal.platform.qemu"
	)

target("project.os-class.lab6.scene1.spinlock")
	set_kind("binary")
	set_enabled(has_config("rvext_a"))

	add_deps("project.os-class.lab6.scheduler")
	add_files("scene1/spinlock.cpp", "linkscript.ld", "scene1/test_and_set.S")

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

target("project.os-class.lab6.scene1.semaphore")
	set_kind("binary")
	set_enabled(has_config("rvext_a"))

	add_deps("project.os-class.lab6.scheduler")
	add_files("scene1/semaphore.cpp", "linkscript.ld")

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

target("project.os-class.lab6.scene2.bad")
	set_kind("binary")
	set_enabled(has_config("rvext_a"))

	add_deps("project.os-class.lab6.scheduler")
	add_files("scene2-bad.cpp", "linkscript.ld")

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

target("project.os-class.lab6.scene2.good")
	set_kind("binary")
	set_enabled(has_config("rvext_a"))

	add_deps("project.os-class.lab6.scheduler")
	add_files("scene2-good.cpp", "linkscript.ld")

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

target("project.os-class.lab6.scene3")
	set_kind("binary")
	set_enabled(has_config("rvext_a"))

	add_deps("project.os-class.lab6.scheduler")
	add_files("scene3.cpp", "linkscript.ld")

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