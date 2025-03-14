
target("os-lab2-helloworld")
	set_kind("binary")
	set_languages("c++23", "c23")

	add_deps("raw-runtime", "os-runtime", "qemu-devices")
	add_files("hello-world.cpp", "$(projectdir)/projects/os-class/raw-linkscript.ld")

	on_run(run_target_qemu)
	after_build(generate_qemu_flash)
