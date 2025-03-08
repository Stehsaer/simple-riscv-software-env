#include "device/uart.hpp"
#include "os/env.hpp"
#include "os/syscall.hpp"
#include "platform-v1/periph.hpp"
#include "platform-v1/sd.hpp"

#include <cmath>
#include <cstring>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <sys/unistd.h>

#include <print>
#include <vector>

namespace elf_parser
{
	using Elf32_Addr = uint32_t;
	using Elf32_Half = uint16_t;
	using Elf32_Off = uint32_t;
	using Elf32_Word = uint32_t;

	struct Elf32_header
	{
		uint8_t e_ident[16];
		Elf32_Half e_type;
		Elf32_Half e_machine;
		Elf32_Word e_version;
		Elf32_Addr e_entry;
		Elf32_Off e_phoff;
		Elf32_Off e_shoff;
		Elf32_Word e_flags;
		Elf32_Half e_ehsize;
		Elf32_Half e_phentsize;
		Elf32_Half e_phnum;
		Elf32_Half e_shentsize;
		Elf32_Half e_shnum;
		Elf32_Half e_shstrndx;

		bool check_magic() const { return e_ident[0] == 0x7F && e_ident[1] == 'E' && e_ident[2] == 'L' && e_ident[3] == 'F'; }
		bool is_riscv() const { return e_machine == 243; }
		bool is_executable() const { return e_type == 2; }
	};

	enum class Segment_type : Elf32_Word
	{
		Null = 0,
		Load = 1,
		Dynamic = 2,
		Interp = 3,
		Note = 4,
		Shlib = 5,
		Phdr = 6,
		Tls = 7,
	};

	struct Segment_flag
	{
	  private:

		Elf32_Word p_flags;

	  public:

		bool r() const { return p_flags & 0x4; }
		bool w() const { return p_flags & 0x2; }
		bool x() const { return p_flags & 0x1; }
	};

	struct Elf32_program_header
	{
		Segment_type p_type;
		Elf32_Off p_offset;
		Elf32_Addr p_vaddr;
		Elf32_Addr p_paddr;
		Elf32_Word p_filesz;
		Elf32_Word p_memsz;
		Segment_flag p_flags;
		Elf32_Word p_align;
	};

	struct Elf32_section_header
	{
		Elf32_Word sh_name;
		Elf32_Word sh_type;
		Elf32_Word sh_flags;
		Elf32_Addr sh_addr;
		Elf32_Off sh_offset;
		Elf32_Word sh_size;
		Elf32_Word sh_link;
		Elf32_Word sh_info;
		Elf32_Word sh_addralign;
		Elf32_Word sh_entsize;
	};
}

class Uart_mount_manager
{
  public:

	FILE *rd, *wr;

	Uart_mount_manager()
	{
		const auto mount_uart_result = os::fs.mount_device("uart1:", platform_v1::uart1.get_device());
		if (mount_uart_result != 0) exit(1);

		rd = freopen("uart1:/", "r", stdin);
		if (rd == nullptr) exit(1);

		wr = freopen("uart1:/", "w", stdout);
		if (wr == nullptr) exit(1);
	}

	~Uart_mount_manager()
	{
		fclose(rd);
		fclose(wr);
		os::fs.unmount_device("uart1:");
	}
};

int main()
{
	Uart_mount_manager uart_mount_manager;

	printf("正在挂载SD卡...\n");
	driver::sd::mount_media_handler();

	while (true)
	{
		driver::sd::reset_sd_library();

		platform_v1::sd::set_speed(400);
		const auto mount_result = driver::fat32::mount_disk();
		if (mount_result.has_value())
		{
			printf("SD卡挂载失败（错误代码%d），重试...\n", mount_result.value());
			sleep(1);
			continue;
		}

		printf("SD卡挂载成功\n");

		const auto err = os::fs.mount_device("sd:", std::make_unique<driver::fat32::Device>());
		if (err != 0)
		{
			printf("无法将SD卡挂载至文件系统（错误代码%d），重试...\n", err);
			sleep(1);
			continue;
		}

		printf("SD卡成功挂载至目录 %s\n", "sd:");

		break;
	}

	platform_v1::sd::set_speed(25000);

	const std::vector<uint8_t> test_data(1024 * 1024 * 64, 199);

	const std::array test_size
		= std::to_array<size_t>({8, 64, 256, 512, 1024, 1024 * 4, 1024 * 8, 1024 * 16, 1024 * 24, 1024 * 32, 1024 * 64});

	for (const auto size : test_size)
	{
		std::println("Testing {} KB", size);
		std::string file_name = std::format("sd:/test_{}KB.bin", size);

		const auto start = clock();

		FILE* file = fopen(file_name.c_str(), "w+b");
		if (file == nullptr)
		{
			std::println("Failed to open file \"{}\", fresult={}", file_name, (int)driver::fat32::last_failure);
			continue;
		}

		fseek(file, 0, SEEK_SET);
		const auto actual_size = fwrite(test_data.data(), 1, size * 1024, file);
		fclose(file);

		if (actual_size != size * 1024)
		{
			std::println("Failed to write {} KB, fresult={}", size, (int)driver::fat32::last_failure);
		}
		else
		{
			std::println("Wrote {} KB in {:.2f}s", size, (float)(clock() - start) / CLOCKS_PER_SEC);
		}
	}

	driver::fat32::unmount_disk();

	return 0;
}