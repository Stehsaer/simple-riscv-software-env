target("module.platform.kintex7")
	set_kind("static")
	set_languages("c++23", "c23", {public=true})

	add_deps("module.device", {public=true})
	add_includedirs("include", {public=true})
	add_files("src/**.cpp")

target_end()

rule("module.platform.kintex7.native")

	on_load(function (target)
		target:add("files", "$(projectdir)/module/platform/kintex7/linkerscript/native.ld")
	end)

	after_build(generate_coe)

rule_end()

rule("module.platform.kintex7.load")

	on_load(function (target)
		target:add("files", "$(projectdir)/module/platform/kintex7/linkerscript/load.ld")
	end)

	after_build(generate_coe)

rule_end()