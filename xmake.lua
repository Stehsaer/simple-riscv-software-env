add_rules("mode.debug", "mode.release", "mode.minsizerel", "mode.releasedbg")

set_toolchains("riscv-gcc-baremetal")
set_config("plat", "bare-metal")

includes("toolchain.lua", "func.lua", "rule.lua")
includes("module", "project")