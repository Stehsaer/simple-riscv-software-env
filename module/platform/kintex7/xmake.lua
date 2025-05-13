includes("device", "fatfs-sd")

rule("generate.coe", function()
	add_deps("generate.bin", {order=true})

	after_link(function(target, opt)
		import("core.project.config")
		import("core.project.depend")
        import("utils.progress")
		
        depend.on_changed(function()
			os.run("python3 $(scriptdir)/bin-to-coe.py %s.bin %s.coe 4", target:targetfile(), target:targetfile())
            progress.show(opt.progress, "${bright blue}generating.$(mode) %s", target:targetfile() .. ".coe")
        end, {files = {target:targetfile()}, changed = target:is_rebuilt()})
	end)

	on_clean(function(target)
		os.rm(target:targetfile() .. ".coe")
	end)
end)


rule("platform.kintex7.native", function()
	on_load(function (target)
		target:add("files", "$(projectdir)/module/platform/kintex7/linkerscript/native.ld")
		target:add("deps", "platform.generic-boot", "platform.kintex7")
	end)

	add_deps("generate.coe", {order=true})
end)

rule("platform.kintex7.nommu-program", function()
	on_load(function (target)
		target:add("files", "$(projectdir)/module/platform/kintex7/linkerscript/nommu-program.ld", "$(projectdir)/module/platform/kintex7/nommu-program/start.S")
		target:add("deps", "platform.kintex7")
	end)
end)