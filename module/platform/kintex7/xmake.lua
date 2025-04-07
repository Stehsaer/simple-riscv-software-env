target("module.platform.kintex7")
	set_kind("static")
	set_languages("c++23", "c23", {public=true})

	add_deps("module.device", {public=true})
	add_includedirs("device/include", {public=true})
	add_files("device/src/**.cpp")

target_end()

rule("generate.coe", function()
	add_deps("generate.bin", {order=true})
	after_build(function(target)
		os.run("python3 $(projectdir)/script/bin-to-coe.py %s.bin %s.coe 4", target:targetfile(), target:targetfile())
	end)
end)


rule("module.platform.kintex7.native", function()
	on_load(function (target)
		target:add("files", "$(projectdir)/module/platform/kintex7/linkerscript/native.ld")
	end)

	add_deps("generate.coe")
end)

rule("module.platform.kintex7.load", function()
	on_load(function (target)
		target:add("files", "$(projectdir)/module/platform/kintex7/linkerscript/load.ld")
	end)

	add_deps("generate.coe")
end)