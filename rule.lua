rule("generate.map", function()
	on_load(function(target)
		target:add("ldflags", "-Map " .. target:targetfile() .. ".map", {force=true})
	end)
end)

rule("generate.bin", function()
	after_build(function(target)
		import("core.project.config")
		local toolchain_prefix = config.get("ccprefix")
		os.run(toolchain_prefix .. "-objcopy -O binary %s %s.bin", target:targetfile(), target:targetfile())
	end)
end)

rule("report-size", function()
	after_build(function(target)
		import("core.project.config")
		local toolchain_prefix = config.get("ccprefix")

		local targetfile = target:targetfile()

		os.exec(toolchain_prefix .. "-size -Gd %s", target:targetfile())
	end)
end)

