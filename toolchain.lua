set_config("ccprefix", "riscv64-unknown-elf")
-- set_config("branch_cost", 5)
set_config("nano_libc", false)

local function parse_arch_string(arch_string)

    local arch = arch_string:match("rv%d%d(.*)")
    local split = arch:split("_")

    local single_letters = {}

    for letter in split[1]:gmatch(".") do
        table.insert(single_letters, letter)
    end

    return (table.concat(single_letters, "_") .. "_" .. table.concat(split, "_", 2)):split("_")
end

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
	set_toolset('ar', toolchain_prefix .. '-gcc-ar')
	set_toolset('sh', toolchain_prefix .. '-ld')

    if is_mode("release") or is_mode("releasedbg") then
        add_cxflags("-funroll-loops", "-finline-functions")
    end

    on_load(function (toolchain)

		import("core.project.config")

        -- Configurations
		local toolchain_prefix = config.get("ccprefix")
		local is_multilib = false
		local branch_cost = config.get("branch_cost")

        function add_cxflags(flags)
            toolchain:add('cxflags', flags)
        end

        function add_cxxflags(flags)
            toolchain:add('cxxflags', flags)
        end

        function add_asflags(flags)
            toolchain:add('asflags', flags)
        end

        function add_ldflags(flags)
            toolchain:add('ldflags', flags)
        end

        function add_arflags(flags)
            toolchain:add('arflags', flags)
        end

        function add_defines(flags)
            toolchain:add('defines', flags)
        end

		function set_toolset(flags)
			toolchain:set('toolset', flags)
		end

        function generate_lib_path(lib_dir)
            if not is_multilib then
                return string.format("%s/%s/%s", lib_dir, get_config('arch'), "ilp32")
            else
                return lib_dir
            end
        end

        function get_compiler_version()
            local version_output = os.iorun(toolchain_prefix .. "-gcc --version")
            local version = version_output:match("%(.-%) (%d+%.%d+%.%d+)")
            return version
        end

        -- Assembler arguments
		add_asflags('-nostartfiles')
        add_asflags('-march=' .. get_config('arch'))
        add_asflags('-mabi=ilp32')
        add_asflags("-Xassembler --defsym -Xassembler " .. ('ARCH_%s=1'):format(get_config('arch'):upper()))

        -- C/C++ Compiler arguments
        add_cxflags('-march=' .. get_config('arch'))
        add_cxflags('-mabi=ilp32')
        -- add_cxflags('-mbranch-cost=' .. tostring(branch_cost))
		add_cxflags("-ffunction-sections", "-fdata-sections", "-fomit-frame-pointer")

        -- Add defines
        local ext_list = parse_arch_string(get_config('arch'))

        for _, ext in ipairs(ext_list) do
            add_defines("RVISA_" .. ext:upper())
        end

        -- Get compiler binary path
        local compiler_path = ""
        if os.is_host("windows") then
            compiler_path = path.directory((os.iorun("where " .. toolchain_prefix .. "-gcc")):split('\n')[1]:gsub("\\", "/"))  .. "/../"
        elseif os.is_host("linux") then
            compiler_path = string.trim(path.directory(os.iorun("which " .. toolchain_prefix .. "-gcc"))) .. "/../"
        end

        -- Add library paths
        local lib_dir_list = {
            string.format("%s/lib", toolchain_prefix),
            string.format("lib/gcc/%s/%s", toolchain_prefix, get_compiler_version()),
        }

        for _, lib_dir in ipairs(lib_dir_list) do
            local lib_path = generate_lib_path(compiler_path .. lib_dir)

            if not os.exists(lib_path) then
                local accepted_subarch = ""

                for _, dir in ipairs(os.dirs(compiler_path .. lib_dir .. "/*")) do

                    local basename = path.basename(dir)
                    
                    if basename:startswith("rv") then
                        accepted_subarch = accepted_subarch .. "-- " .. path.basename(dir) .. '\n'
                    end
                end
                
                raise(string.format("Incorrect subarch string \"%s\". \nAccepted subarchs:\n%s", get_config('arch'), accepted_subarch))
            end

            add_ldflags('-L' .. lib_path)
        end

        -- General linker flags
        add_ldflags("--oformat=elf32-littleriscv")
        add_ldflags("--gc-sections -(")
    end)

toolchain_end()
