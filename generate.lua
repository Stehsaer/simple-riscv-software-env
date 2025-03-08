-- Common function for generating BIN and COE files after build
function generate_coe(target) 
	import("core.project.config")

	local toolchain_prefix = config.get("ccprefix")

    os.run(toolchain_prefix .. "-objcopy -O binary %s %s.bin", target:targetfile(), target:targetfile())
    os.run("python3 $(projectdir)/script/bin-to-coe.py %s.bin %s.coe 4", target:targetfile(), target:targetfile())
    os.exec(toolchain_prefix .. "-size -Gd %s", target:targetfile())
end

-- Common function for generating HEX files after build
function generate_hex(target) 
	import("core.project.config")

	local toolchain_prefix = config.get("ccprefix")

    os.run(toolchain_prefix .. "-objcopy -O ihex %s %s.hex", target:targetfile(), target:targetfile())
    os.exec(toolchain_prefix .. "-size -Gd %s", target:targetfile())
end