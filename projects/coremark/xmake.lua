target("coremark")
	set_kind("binary")
	
	add_deps("loaded-runtime", "platform-v1")
	add_files("*.cpp", "*.c", "$(projectdir)/template/loaded-linkscript.ld")

	on_config(
		function (target)

			import("core.tool.compiler")

			local flags = compiler.compflags("a.c", {target = target})

			local flag_str = ""
			for _, flag in ipairs(flags) do
				flag_str = flag_str .. " " .. flag
			end

			target:add("defines",
				"ITERATIONS=8000", 
				"COMPILER_FLAGS=\"" .. flag_str .. "\"",
				"PERFORMANCE_RUN=1", 
				"COMPILER_VERSION=\"" .. target:tool("cc") .. "\"" 
			)
    	end
	)
