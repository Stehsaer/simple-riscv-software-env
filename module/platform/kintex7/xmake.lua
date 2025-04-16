includes("device", "fatfs-sd")

rule("generate.coe", function()
	add_deps("generate.bin", {order=true})
	after_build(function(target)
		os.run("python3 $(scriptdir)/bin-to-coe.py %s.bin %s.coe 4", target:targetfile(), target:targetfile())
	end)
end)


rule("platform.kintex7.native", function()
	on_load(function (target)
		target:add("files", "$(projectdir)/module/platform/kintex7/linkerscript/native.ld")
		target:add("deps", "platform.generic-boot", "platform.kintex7")
	end)

	add_deps("generate.coe")
end)

rule("platform.kintex7.nommu-program", function()
	on_load(function (target)
		target:add("files", "$(projectdir)/module/platform/kintex7/linkerscript/nommu-program.ld", "$(projectdir)/module/platform/kintex7/nommu-program/start.S")
		target:add("deps", "platform.kintex7")
	end)
end)