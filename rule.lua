rule("generate.map", function()
	on_load(function(target)
		target:add("ldflags", "-Map " .. target:targetfile() .. ".map", {force=true})
	end)

	on_clean(function(target)
		os.rm(target:targetfile() .. ".map")
	end)
end)

rule("generate.bin", function()
	after_link(function(target, opt)
		import("core.project.config")
		import("core.project.depend")
        import("utils.progress")
		
        depend.on_changed(function()
			local toolchain_prefix = config.get("ccprefix")
			os.run(toolchain_prefix .. "-objcopy -O binary %s %s.bin", target:targetfile(), target:targetfile())
            progress.show(opt.progress, "${bright blue}generating.$(mode) %s", target:targetfile() .. ".bin")
        end, {files = {target:targetfile()}, changed = target:is_rebuilt()})
	end)

	on_clean(function(target)
		os.rm(target:targetfile() .. ".bin")
	end)
end)

rule("report-size", function()
	after_build(function(target)
		import("core.project.config")
		local toolchain_prefix = config.get("ccprefix")

		local targetfile = target:targetfile()

		local output, _ = os.iorun(toolchain_prefix .. "-size --format=Berkeley %s", target:targetfile())

		local data = output:split("\n")[2]:split("%s+")

		local text_size = tonumber(data[1])
		local data_size = tonumber(data[2])
		local bss_size = tonumber(data[3])

		cprint("╭─${bright white}[Size Report] ${clear}(%s)", target:name())
		cprint("│◦ text  : %d \t(%dK + %d)", text_size, math.floor(text_size / 1024), text_size % 1024)
		cprint("│◦ data  : %d \t(%dK + %d)", data_size, math.floor(data_size / 1024), data_size % 1024)
		cprint("│◦ bss   : %d \t(%dK + %d)", bss_size, math.floor(bss_size / 1024), bss_size % 1024)
		cprint("╰▻ ${cyan}Binary${clear}: %d \t(%dK + %d)", text_size + data_size, math.floor((text_size + data_size) / 1024), (text_size + data_size) % 1024)
	end)
end)