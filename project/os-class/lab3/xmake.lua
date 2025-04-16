target("project.os-class.lab3.disk-io")

	set_kind("binary")
	add_files("disk-io.cpp")
	add_deps(
		"platform.qemu", 
		"platform.generic-boot",
		"file.interface",
		"file.driver.uart",
		"file.driver.fatfs.backend.virtio"
	)

	add_rules(
		"platform.qemu.native",
		"report-size"
	)

	-- set_runargs("-S -s")

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

target("project.os-class.lab3.boot")

	set_kind("binary")
	add_files("boot.cpp")
	add_deps(
		"platform.qemu", 
		"platform.generic-boot",
		"file.interface",
		"file.driver.uart",
		"file.driver.fatfs.backend.virtio"
	)
	
	add_rules(
		"platform.qemu.native",
		"report-size"
	)

	-- set_runargs("-S -s")

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

target("project.os-class.lab3.test")

	set_kind("binary")
	add_files("test.ld", "test.S")