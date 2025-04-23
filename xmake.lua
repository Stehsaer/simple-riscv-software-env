add_rules("mode.debug", "mode.release", "mode.minsizerel", "mode.releasedbg")

if is_mode("release") or is_mode("releasedbg") then
	add_rules("c++.unity_build")
end

set_toolchains("riscv-gcc-baremetal")
set_config("plat", "bare-metal")

set_allowedplats("bare-metal")

add_repositories("riscv-repo riscv-repo")

includes("toolchain.lua", "rule.lua")
includes("module", "project")