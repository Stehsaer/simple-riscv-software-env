set_config("ccprefix", "riscv64-unknown-elf")
-- set_config("branch_cost", 5)
set_config("nano_libc", false)

isa_arch_list = {}

function has_isa(arch)
    return table.contains(isa_arch_list, arch)
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

        local function parse_arch_string(arch_string)
            local arch_match = arch_string:match("rv%d%d(.*)")

            if arch_match == nil then
                return nil
            end

            local split_result = arch_match:split("_")

            local single_letters = {}

            for letter in split_result[1]:gmatch(".") do
                table.insert(single_letters, letter)
            end

            local concated_table = (table.concat(single_letters, "_") .. "_" .. table.concat(split_result, "_", 2)):split(
                "_")

            -- Atomic

            if table.contains(concated_table, "a") and not table.contains(concated_table, "zaamo") then
                table.insert(concated_table, "zaamo")
            end
            if table.contains(concated_table, "a") and not table.contains(concated_table, "zalrsc") then
                table.insert(concated_table, "zalrsc")
            end
            if table.contains(concated_table, "zaamo") and table.contains(concated_table, "zalrsc") and not table.contains(concated_table, "a") then
                table.insert(concated_table, "a")
            end

            table.sort(concated_table)
            return concated_table
        end

        local function get_table_length(t)
            local count = 0
            for a, b in pairs(t) do
                if b ~= nil then
                    count = count + 1
                end
            end
            return count
        end

        local function arch_table_equal(table1, table2)
            if get_table_length(table1) ~= get_table_length(table2) then
                return false
            end

            for _, value in ipairs(table1) do
                if not table.contains(table2, value) then
                    return false
                end
            end

            return true
        end

        local function get_lib_path(arch_string, compiler_path, lib_dir)
            local arch = parse_arch_string(arch_string)
            local dirs = os.dirs(compiler_path .. lib_dir .. "/*")
            local filtered_dirs = {}

            for _, dir in ipairs(dirs) do
                local basename = path.basename(dir)
                local parsed = parse_arch_string(basename)
                if parsed ~= nil then
                    filtered_dirs[dir] = parsed
                end
            end

            for dir, parse in pairs(filtered_dirs) do
                if arch_table_equal(arch, parse) then
                    return dir .. "/ilp32"
                end
            end

            return nil
        end

        local function get_compiler_version(toolchain_prefix)
            local version_output = os.iorun(toolchain_prefix .. "-gcc --version")
            local version = version_output:match("%(.-%) (%d+%.%d+%.%d+)")
            return version
        end
        
        -- Add defines
        local ext_list = parse_arch_string(arch_string)
        for _, ext in ipairs(ext_list) do
            add_defines("RVISA_" .. ext:upper())
        end
        
        add_asflags('-nostartfiles')
        add_asflags('-march=' .. arch_string)
        add_asflags('-mabi=ilp32')
        add_cxflags('-march=' .. arch_string)
        add_cxflags('-mabi=ilp32')
        add_cxflags("-ffunction-sections", "-fdata-sections", "-fomit-frame-pointer")

        isa_arch_list = parse_arch_string(arch_string)

        -- Get compiler binary path
        local compiler_path = ""
        if os.is_host("windows") then
            compiler_path = path.directory((os.iorun("where " .. toolchain_prefix .. "-gcc")):split('\n')[1]:gsub("\\", "/"))  .. "/../"
        elseif os.is_host("linux") then
            compiler_path = string.trim(path.directory(os.iorun("which " .. toolchain_prefix .. "-gcc"))) .. "/../"
        end
        
        -- Add library paths
        local compiler_version = get_compiler_version(toolchain_prefix)
        if compiler_version == nil then
            raise("Compiler version not found. Please check the toolchain prefix.")
        end
        
        local lib_dir_list = {
            string.format("%s/lib", toolchain_prefix),
            string.format("lib/gcc/%s/%s", toolchain_prefix, compiler_version),
        }

        for _, lib_dir in ipairs(lib_dir_list) do
            local lib_path = get_lib_path(arch_string, compiler_path, lib_dir)
            if lib_path == nil then
                raise("Cannot find library path for arch: " .. arch_string)
            end
            add_ldflags('-L' .. lib_path)
        end

        -- General linker flags
        add_ldflags("-nostdlib --oformat=elf32-littleriscv")
        add_ldflags("--gc-sections -(")
    end)

toolchain_end()
