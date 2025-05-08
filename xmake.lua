add_rules("mode.debug", "mode.release", "mode.minsizerel", "mode.releasedbg")

includes("toolchain.lua", "rule.lua")

set_toolchains("riscv-gcc-baremetal")
set_config("plat", "bare-metal")

set_allowedplats("bare-metal")

add_repositories("riscv-repo riscv-repo")

includes("module", "project")