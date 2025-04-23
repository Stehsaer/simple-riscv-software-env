package("ffmpeg-riscv-baremetal")

	add_urls("https://github.com/FFmpeg/FFmpeg.git")
	add_versions("7.1", "f459c56b0fe3b35f1e21951791df0f111d1dbbcf" )

	on_load(function(package)
		package:add("cflags", "-Wno-incompatible-pointer-types")
	end)

	-- add_patches("7.1", path.join(os.scriptdir(), "fix.patch"))

	on_install("bare-metal", function(package)

		local disable_list = {
			"doc", 
			"avdevice", 
			"postproc", 
			"pthreads", 
			"network", 
			"hwaccels", 
			"rvv", 
			"zlib", 
			"bzlib", 
			"iconv", 
			"lzma", 
			"programs", 
			"cuda-llvm", 
			"asm", 
			"runtime-cpudetect", 
			"devices", 
			"protocols"
		}

		local extra_libpath_str = ""

		local extra_libpath = {
			"/opt/riscv/riscv64-unknown-elf/lib/rv32im_zicond_zicsr_zifencei/ilp32",
			"/opt/riscv/lib/gcc/riscv64-unknown-elf/14.2.0/rv32im_zicond_zicsr_zifencei/ilp32/"
		}
		for _, path in ipairs(extra_libpath) do
			extra_libpath_str = extra_libpath_str .. " -L" .. path
		end

		local extra_libs_str = ""

		for _, lib in ipairs(extra_libs) do
			extra_libs_str = extra_libs_str .. " -l" .. lib
		end

		local extra_flags = {
			"--arch=riscv32",
			"--cpu=rv32im_zicond_zicsr_zifencei",
			"--target-os=none",
			"--enable-cross-compile",
			"--ld=riscv64-unknown-elf-ld",
			"--cc=riscv64-unknown-elf-gcc",
			"--ar=riscv64-unknown-elf-ar",
			"--cxx=riscv64-unknown-elf-g++",
			"--cross-prefix=riscv64-unknown-elf-",
			"--extra-cflags=-Wno-incompatible-pointer-types -finline-functions",
			"--extra-cxxflags=-finline-functions",
			"--extra-ldflags=-static --no-gc-sections",
			"--extra-libs=-lgloss",
			"--enable-protocol=file"
		}

		local config = {}

		for _, name in ipairs(disable_list) do
			table.insert(config, "--disable-" .. name)
		end

		for _, name in ipairs(extra_flags) do
			table.insert(config, name)
		end

		import("package.tools.autoconf").install(package, config)
	end)