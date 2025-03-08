add_rules("mode.debug", "mode.release", "mode.minsizerel", "mode.releasedbg")

set_toolchains("riscv")
set_config("plat", "bare-metal")

includes("toolchain.lua", "generate.lua")
includes("runtime", "library", "projects")