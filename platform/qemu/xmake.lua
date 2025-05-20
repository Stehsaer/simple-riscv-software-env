target("platform.qemu")
	set_kind("static")
	set_languages("c++23", {public=true})

	add_files("src/*.cpp")
	add_deps("platform.common-headers", {public=true})
	add_includedirs("include", {public=true})
target_end()

rule("generate.qemu-flash", function()
	add_deps("generate.bin", {order=true})
	after_build(function(target)
		import("core.project.config")
		local toolchain_prefix = config.get("ccprefix")
		os.run(toolchain_prefix .. "-objcopy -O binary %s %s.bin", target:targetfile(), target:targetfile())
		os.run("python3 $(scriptdir)/fill-flash.py %s.bin %s.bin", target:targetfile(), target:targetfile())
	end)
end)

rule("platform.qemu.run")

	on_run(function(target)
		local argv = import("core.base.option").get("arguments")

		local extra_args = ""
		local bin_path = ""

		for _, v in ipairs(argv) do
			if v == "--debug" then
				extra_args = extra_args .. " -s -S"
			end
			if v:startswith("--bin=") then
				bin_path = v:sub(6)
			end
		end

		if bin_path == "" then
			bin_path = "$(projectdir)/disk.img"
		end

		local cmd_prefix = "qemu-system-riscv32 -M virt -cpu rv32,zicond=true -nographic -bios none -m 2G -global virtio-mmio.force-legacy=false"
		local bootloader_args = " -drive file=\"" .. target:targetfile() .. ".bin\",format=raw,if=pflash"
		local disk_args = " -device virtio-blk-device,drive=hd0 -drive file=" .. bin_path .. ",if=none,format=raw,id=hd0,media=disk,index=0,cache=none"
		local run_str = cmd_prefix .. bootloader_args .. disk_args .. " " .. extra_args
		os.exec(run_str)
	end)

rule_end()

rule("platform.qemu.config", function()
	on_load(function(target)
		target:add("ldflags", "-L$(projectdir)/platform/qemu")
	end)
end)