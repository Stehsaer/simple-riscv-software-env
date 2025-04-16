#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <optional>
#include <print>
#include <string>
#include <system_error>
#include <vector>

#include <file/driver/fatfs/backend/virtio.hpp>
#include <file/driver/uart.hpp>
#include <file/interface.hpp>
#include <platform/qemu.hpp>

constexpr size_t min_address = 0x8010'0000;
constexpr size_t max_address = 0xFFF0'0000;

#define ANSI_ERR_COLOR "\033[91m"
#define ANSI_GREEN_COLOR "\033[92m"
#define ANSI_WARNING_COLOR "\033[93m"
#define ANSI_BOLD "\033[1m"
#define ANSI_RESET_COLOR "\033[0m"

#define ERROR_PREFIX_STRING ANSI_ERR_COLOR ANSI_BOLD "Error:" ANSI_RESET_COLOR " "

using namespace device::qemu::virtio;

class File_raii
{
	FILE* file;

  public:

	File_raii(const char* name, const char* mode) { file = fopen(name, mode); }

	bool success() const { return file != nullptr; }

	~File_raii()
	{
		if (file) fclose(file);
	}

	operator FILE*() { return file; }
};

namespace elf
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

struct Program_load_result
{
	uint32_t entry_address;

	int execute(int argc, char** argv) const
	{
		using Program_type = int (*)(int argc, char** argv);
		return ((Program_type)entry_address)(argc, argv);
	}
};

std::optional<Program_load_result> load_file(const char* path)
{
	char name[512];
	sprintf(name, "%s", path);

	printf("Loading Program \"%s\"\n", name);

	File_raii file(name, "r");

	if (!file.success())
	{
		printf(ERROR_PREFIX_STRING "Failed to open file \"%s\"\n", name);
		return std::nullopt;
	}

	elf::Elf32_header elf_header;
	if (!fread(&elf_header, sizeof(elf::Elf32_header), 1, file))
	{
		printf(ERROR_PREFIX_STRING "Failed to read ELF header\n");
		return std::nullopt;
	}

	if (!elf_header.check_magic())
	{
		printf(ERROR_PREFIX_STRING "Not a valid ELF file\n");
		printf(
			"-- Detail: Expected magic number 7F 45 4C 46, got %02X %02X %02X %02X\n",
			elf_header.e_ident[0],
			elf_header.e_ident[1],
			elf_header.e_ident[2],
			elf_header.e_ident[3]
		);
		return std::nullopt;
	}

	if (!elf_header.is_riscv())
	{
		printf(ERROR_PREFIX_STRING "Mismatched architecture: not RISCV\n");
		return std::nullopt;
	}

	if (!elf_header.is_executable())
	{
		printf(ERROR_PREFIX_STRING "Not an executable ELF file\n");
		return std::nullopt;
	}

	if (elf_header.e_phentsize != sizeof(elf::Elf32_program_header))
	{
		printf(ERROR_PREFIX_STRING "Unknown ELF format\n");
		return std::nullopt;
	}

	if (elf_header.e_phnum == 0)
	{
		printf(ERROR_PREFIX_STRING "Empty ELF program header\n");
		return std::nullopt;
	}

	size_t total_size = 0;
	elf::Elf32_program_header program_header;

	for (int i = 0; i < elf_header.e_phnum; i++)
	{
		fseek(file, elf_header.e_phoff + i * sizeof(elf::Elf32_program_header), SEEK_SET);

		if (!fread(&program_header, sizeof(elf::Elf32_program_header), 1, file))
		{
			printf(ERROR_PREFIX_STRING "Failed to read program header\n");
			return std::nullopt;
		}

		switch (program_header.p_type)
		{
		case elf::Segment_type::Load:
		case elf::Segment_type::Tls:
		{
			if (program_header.p_filesz > program_header.p_memsz)
			{
				printf(ERROR_PREFIX_STRING "Invalid segment: file size exceeds memory size\n");
				return std::nullopt;
			}

			if (program_header.p_memsz == 0)
			{
				printf(ERROR_PREFIX_STRING "Segment error: memory size is 0\n");
				return std::nullopt;
			}

			if (program_header.p_vaddr < min_address || program_header.p_vaddr + program_header.p_memsz >= max_address)
			{
				printf(ERROR_PREFIX_STRING "Segment error: memory address out of range\n");
				printf(
					"-- Detail:\n  -- Acceptable range: 0x%08X - 0x%08X\n  -- Segment range: 0x%08X - 0x%08X\n",
					min_address,
					max_address,
					program_header.p_vaddr,
					program_header.p_vaddr + program_header.p_memsz
				);
				return std::nullopt;
			}

			printf(
				"SEGMENT [%d]: %s, %c%c%c %d Bytes --> 0x%08X\n",
				i,
				program_header.p_type == elf::Segment_type::Load ? "Load" : "TLS",
				program_header.p_flags.r() ? 'R' : '-',
				program_header.p_flags.w() ? 'W' : '-',
				program_header.p_flags.x() ? 'X' : '-',
				program_header.p_filesz,
				program_header.p_vaddr
			);

			fseek(file, program_header.p_offset, SEEK_SET);

			if (!fread((void*)(size_t)program_header.p_vaddr, program_header.p_filesz, 1, file))
			{
				printf(ERROR_PREFIX_STRING "Failed to read segment data\n");
				return std::nullopt;
			}

			memset(
				(void*)(size_t)(program_header.p_vaddr + program_header.p_filesz),
				0,
				program_header.p_memsz - program_header.p_filesz
			);

			total_size += program_header.p_filesz;

			break;
		}
		default:
			printf("SEGMENT [%d]: Not loadable, skipping..\n", i);
			continue;
		}
	}

	asm volatile("fence.i");

	printf("Program loaded successfully!\n-- Entry point: 0x%08X\n-- Size: %d Bytes\n", elf_header.e_entry, total_size);

	return Program_load_result{elf_header.e_entry};
}

IO* find_device()
{
	std::vector<std::reference_wrapper<IO>> available_devices;

	for (auto i = 0u; i < 8; i++)
	{
		auto& virtio = IO::at(i);

		if (!virtio.check_magic_value()) continue;
		if (virtio.version != 2) continue;
		if (virtio.device_id != 2) continue;

		available_devices.emplace_back(virtio);
	}

	if (available_devices.empty()) return nullptr;

	if (available_devices.size() > 1)
	{
		std::println("Multiple Virtio devices found!");
		return nullptr;
	}
	else
	{
		return &available_devices[0].get();
	}
}

int main()
{
	auto& uart = platform::qemu::uart;
	file::fs.mount_device("uart:/", std::make_unique<file::driver::qemu::Uart_driver>(uart));

	freopen("uart:/", "w", stdout);

	auto* virtio = find_device();

	if (virtio == nullptr)
	{
		printf(ERROR_PREFIX_STRING "Virtio device not found\n");
		return 1;
	}

	std::println("Found Virtio device at 0x{:08X}", (size_t)virtio);

	file::driver::fatfs::media_interface = std::make_unique<file::driver::fatfs::backend::virtio::Media_interface>(*virtio);

	while (true)
	{
		const auto mount_disk_result = file::driver::fatfs::mount_disk();
		if (mount_disk_result.has_value())
		{
			std::println(ERROR_PREFIX_STRING "Failed to mount disk: {}, retrying", (int)mount_disk_result.value());
			continue;
		}

		break;
	}

	const auto mount_result = file::fs.mount_device("virtio:/", std::make_unique<file::driver::fatfs::Device>());

	if (mount_result != 0)
	{
		std::println(ERROR_PREFIX_STRING "Failed to mount Virtio device: {}", mount_result);
		return 1;
	}

	const auto program = load_file("virtio:/init.elf");
	if (!program.has_value())
	{
		std::println(ERROR_PREFIX_STRING "Failed to load program");
		return 1;
	}

	const auto result = program->execute(0, nullptr);
	if (result != 0)
	{
		std::println(ERROR_PREFIX_STRING "Program exited with error code: {}", result);
		return 1;
	}

	return 0;
}
