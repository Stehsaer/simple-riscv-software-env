add_rules("mode.debug", "mode.release", "mode.minsizerel", "mode.releasedbg")
add_rules("plugin.compile_commands.autoupdate", {outputdir = "build"})

if is_mode("minsizerel") then
	set_symbols("debug")
end

includes("toolchain.lua", "rule.lua")

set_toolchains("riscv-gcc-baremetal")
set_config("plat", "bare-metal")
set_policy("check.auto_ignore_flags", false)

set_allowedplats("bare-metal")

add_repositories("riscv-repo riscv-repo")

includes("platform", "baremetal", "boot", "project")