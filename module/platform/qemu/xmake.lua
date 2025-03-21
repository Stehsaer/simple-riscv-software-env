target("module.platform.qemu")
	set_kind("static")
	set_languages("c++23", "c23", {public=true})

	add_deps("module.device", {public=true})
	add_includedirs("include", {public=true})
	add_files("src/**.cpp")

target_end()

rule("module.platform.qemu.native")

	on_load(function (target)
		target:add("files", "$(projectdir)/module/platform/qemu/linkerscript/native.ld")
	end)

	after_build(generate_qemu_flash)

rule_end()
