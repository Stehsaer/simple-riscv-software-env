set_config("ccprefix", "riscv64-unknown-elf")
-- set_config("branch_cost", 5)
set_config("nano_libc", false)

toolchain("riscv-gcc-baremetal")

    set_kind('standalone')

    -- Add syslinks to standard libraries

    local libm_is_nano = is_mode("minsizerel")
    local libc_is_nano = is_mode("minsizerel") or get_config("nano_libc")

    add_syslinks(libm_is_nano and "m_nano" or "m")
    add_syslinks(libc_is_nano and "c_nano" or "c")
    add_syslinks("gcc", "stdc++")

	local toolchain_prefix = get_config("ccprefix")
	if toolchain_prefix == nil then
		toolchain_prefix = "riscv64-unknown-elf"
	end

	set_toolset('cc', toolchain_prefix .. '-gcc')
	set_toolset('cxx', toolchain_prefix .. '-g++')
	set_toolset('cpp', toolchain_prefix .. '-g++')
	set_toolset('as', toolchain_prefix .. '-g++')
	set_toolset('strip', toolchain_prefix .. '-strip')
	set_toolset('ld', toolchain_prefix .. '-ld')
	set_toolset('ar', toolchain_prefix .. '-ar')
	set_toolset('sh', toolchain_prefix .. '-ld')

    if is_mode("release") or is_mode("releasedbg") then
        add_cxflags("-funroll-loops")
    end

    on_load(function (toolchain)

		import("core.project.config")

        -- Configurations
		local toolchain_prefix = config.get("ccprefix")
        local arch_string = config.get("arch")

        local function add_cxflags(flags)
            toolchain:add('cxflags', flags)
        end

        local function add_cxxflags(flags)
            toolchain:add('cxxflags', flags)
        end

        local function add_asflags(flags)
            toolchain:add('asflags', flags)
        end

        local function add_ldflags(flags)
            toolchain:add('ldflags', flags)
        end

        local function add_defines(flags)
            toolchain:add('defines', flags)
        end

        import("lua_func.parse_arch_string", {alias="parse_arch_string"})
        
        -- Add defines
        local ext_list = parse_arch_string(arch_string)
        for _, ext in ipairs(ext_list) do
            add_defines("RVISA_" .. ext:upper())
            config.set("rvext_"..ext, true)
        end
        
        add_asflags('-nostartfiles')
        add_asflags('-march=' .. arch_string)
        add_asflags('-mabi=ilp32')
        add_cxflags('-march=' .. arch_string)
        add_cxflags('-mabi=ilp32')
        add_cxflags("-ffunction-sections", "-fdata-sections", "-fomit-frame-pointer")

        local libgcc_path = path.directory(os.iorun(string.format("%s-gcc -march=%s -mabi=ilp32 --print-libgcc-file-name", toolchain_prefix, arch_string)))
        local multi_directory = string.trim(os.iorun(string.format("%s-gcc -march=%s -mabi=ilp32 --print-multi-directory", toolchain_prefix, arch_string)))
        local sysroot = string.trim(os.iorun(string.format("%s-gcc -march=%s -mabi=ilp32 --print-sysroot", toolchain_prefix, arch_string)))

        add_ldflags(string.format("-L%s", libgcc_path))
        add_ldflags(string.format("-L%s/lib/%s", sysroot, multi_directory))
        add_ldflags("-nostdlib --oformat=elf32-littleriscv")
        add_ldflags("--gc-sections -(")
    end)

toolchain_end()
