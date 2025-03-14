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

function generate_qemu_flash(target)
    import("core.project.config")

	local toolchain_prefix = config.get("ccprefix")

    os.run(toolchain_prefix .. "-objcopy -O binary %s %s.bin", target:targetfile(), target:targetfile())
    os.run("python3 $(projectdir)/projects/os-class/fill-flash.py %s.bin %s.bin", target:targetfile(), target:targetfile())
end

function run_target_qemu(target) 
	os.exec("qemu-system-riscv32 -M virt -cpu rv32,zicond=true -nographic -bios none -drive file=\"" .. target:targetfile() .. ".bin\",format=raw,if=pflash -S -s -m 2G")
end
