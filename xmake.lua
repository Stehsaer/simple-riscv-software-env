add_rules("mode.debug", "mode.release", "mode.minsizerel", "mode.releasedbg")

set_toolchains("riscv-gcc")
set_config("plat", "bare-metal")

includes("toolchain.lua", "func.lua")
includes("runtime", "library", "projects")