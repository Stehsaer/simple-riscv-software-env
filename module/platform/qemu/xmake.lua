target("module.platform.qemu")
	set_kind("static")
	set_languages("c++23", "c23", {public=true})

	add_deps("module.device", {public=true})
	add_includedirs("include", {public=true})
	add_files("src/**.cpp")

target_end()

rule("generate.qemu-flash", function()
	add_deps("generate.bin", {order=true})
	after_build(function(target)
		import("core.project.config")
		local toolchain_prefix = config.get("ccprefix")
		os.run(toolchain_prefix .. "-objcopy -O binary %s %s.bin", target:targetfile(), target:targetfile())
		os.run("python3 $(projectdir)/script/fill-flash.py %s.bin %s.bin", target:targetfile(), target:targetfile())
	end)
end)

rule("run-qemu", function()
	add_deps("generate.qemu-flash", {order=true})
	on_run(function(target)
		local run_str = "qemu-system-riscv32 -M virt -cpu rv32,zicond=true -nographic -bios none -drive file=\"" .. target:targetfile() .. ".bin\",format=raw,if=pflash -m 2G"
		os.exec(run_str)
	end)
end)


rule("module.platform.qemu.native")
	on_load(function (target)
		target:add("files", "$(projectdir)/module/platform/qemu/linkerscript/native.ld")
	end)
	add_deps("run-qemu")
rule_end()
