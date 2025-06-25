target("project.os-class.lab8.system")
	set_kind("binary")
	set_languages("c++23")

	add_deps(
		"baremetal.platform.qemu"
	)

	add_includedirs("include")
	add_files("src/*.cpp", "src/*.S")

	add_cxflags("-fno-exceptions")

	if is_mode("debug") then
		add_defines("_DEBUG")
	end

	add_rules(
		"generate.qemu-flash",
		"platform.qemu.config",
		"platform.qemu.run.disk",
		"link.stage0",
		"report-size"
	)
