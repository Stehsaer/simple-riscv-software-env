target("project.os-class.lab4.main")

	set_kind("binary")
	add_files("main.cpp")
	add_files("*.S")
	add_deps(
		"platform.qemu", 
		"platform.generic-boot",
		"file.interface",
		"file.driver.uart"
	)

	add_rules(
		"platform.qemu.native",
		"report-size",
		"platform.qemu.run"
	)

	on_run(function(target)

		local argv = target:get("runargs")

		if argv == nil then
			argv = ""
		end

		local cmd_prefix = "qemu-system-riscv32 -M virt -cpu rv32,zicond=true -nographic -bios none -m 2G -global virtio-mmio.force-legacy=false"
		local bootloader_args = " -drive file=\"" .. target:targetfile() .. ".bin\",format=raw,if=pflash"
		local disk_args = " -device virtio-blk-device,drive=hd0 -drive file=$(projectdir)/disk.img,if=none,format=raw,id=hd0,media=disk,index=0"
		local run_str = cmd_prefix .. bootloader_args .. disk_args .. " " .. argv
		
		os.exec(run_str)
	end)