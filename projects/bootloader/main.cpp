#include "driver/fat32.hpp"
#include "os/env.hpp"
#include "platform-v1/periph.hpp"
#include "platform-v1/sd.hpp"

#include "elf.hpp"

#include <cstring>
#include <optional>
#include <string>
#include <system_error>

#define CLOCK_FREQ 200'000'000
#define BAUD_RATE 1000000

constexpr size_t min_address = 0x8010'0000;
constexpr size_t max_address = 0xFFF0'0000;

#define ANSI_ERR_COLOR "\033[91m"
#define ANSI_GREEN_COLOR "\033[92m"
#define ANSI_WARNING_COLOR "\033[93m"
#define ANSI_BOLD "\033[1m"
#define ANSI_RESET_COLOR "\033[0m"

#define ERROR_PREFIX_STRING ANSI_ERR_COLOR ANSI_BOLD "Error:" ANSI_RESET_COLOR " "

const char* fresult_string[]
	= {[FR_OK] = "FR_OK",
	   [FR_DISK_ERR] = "FR_DISK_ERR",
	   [FR_INT_ERR] = "FR_INT_ERR",
	   [FR_NOT_READY] = "FR_NOT_READY",
	   [FR_NO_FILE] = "FR_NO_FILE",
	   [FR_NO_PATH] = "FR_NO_PATH",
	   [FR_INVALID_NAME] = "FR_INVALID_NAME",
	   [FR_DENIED] = "FR_DENIED",
	   [FR_EXIST] = "FR_EXIST",
	   [FR_INVALID_OBJECT] = "FR_INVALID_OBJECT",
	   [FR_WRITE_PROTECTED] = "FR_WRITE_PROTECTED",
	   [FR_INVALID_DRIVE] = "FR_INVALID_DRIVE",
	   [FR_NOT_ENABLED] = "FR_NOT_ENABLED",
	   [FR_NO_FILESYSTEM] = "FR_NO_FILESYSTEM",
	   [FR_MKFS_ABORTED] = "FR_MKFS_ABORTED",
	   [FR_TIMEOUT] = "FR_TIMEOUT",
	   [FR_LOCKED] = "FR_LOCKED",
	   [FR_NOT_ENOUGH_CORE] = "FR_NOT_ENOUGH_CORE",
	   [FR_TOO_MANY_OPEN_FILES] = "FR_TOO_MANY_OPEN_FILES",
	   [FR_INVALID_PARAMETER] = "FR_INVALID_PARAMETER"};

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

class Directory_raii
{
	DIR dir;
	FRESULT result;

  public:

	Directory_raii(const char* name) { result = f_opendir(&dir, name); }

	bool success() const { return result == FR_OK; }
	const char* fail_string() const { return fresult_string[result]; }

	~Directory_raii()
	{
		if (success()) f_closedir(&dir);
	}

	operator DIR&() { return dir; }
};

void command_ls()
{
	Directory_raii dir("/");

	if (!dir.success())
	{
		printf(ERROR_PREFIX_STRING "Unable to read directory (%s)\n", dir.fail_string());
		return;
	}

	while (true)
	{
		auto [entry_result, entry] = driver::fat32::readdir(dir);

		if (entry_result != FR_OK)
		{
			printf(ERROR_PREFIX_STRING "Unable to acquire entry info (%s)\n", fresult_string[entry_result]);
			break;
		}

		if (entry.fname[0] == 0) break;

		if (entry.fattrib & AM_DIR)
		{
			printf("%s <DIR>\n", entry.fname);
		}
		else
		{
			printf("%s <FILE> %d Bytes\n", entry.fname, entry.fsize);
		}
	}
}

void command_info()
{
	printf("CPU Frequency: %dkHz\n", (int)(platform_v1::frequency / 1'000));
	printf("Serial Port Baudrate: %d\n", BAUD_RATE);
}

struct Program_load_result
{
	uint32_t entry_address;
};

std::optional<Program_load_result> load_file(const char* path)
{
	char name[512];
	sprintf(name, "sd:/%s", path);

	printf("Loading Program \"%s\"\n", name);

	File_raii file(name, "r");

	if (!file.success())
	{
		printf(ERROR_PREFIX_STRING "Failed to open file \"%s\"\n", name);
		return std::nullopt;
	}

	elf::Elf32_header elf_header;
	fread(&elf_header, sizeof(elf::Elf32_header), 1, file);

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

		fread(&program_header, sizeof(elf::Elf32_program_header), 1, file);

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

			if (program_header.p_filesz == 0)
			{
				printf(ERROR_PREFIX_STRING "Segment error: file size is 0\n");
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
			fread((void*)(size_t)program_header.p_vaddr, program_header.p_filesz, 1, file);
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

void mount_sd_card()
{
	printf("Mounting SD card...\n");
	driver::sd::mount_media_handler();

	while (true)
	{
		driver::sd::reset_sd_library();
		platform_v1::sd::set_speed(400);  // Low speed mode

		const auto mount_result = driver::fat32::mount_disk();
		if (mount_result.has_value())
		{
			printf(ERROR_PREFIX_STRING "Failed to mount SD card (%s). Retrying...\n", fresult_string[mount_result.value()]);
			sleep(1);
			continue;
		}

		printf("SD card mounted!\n");

		const auto err = os::fs.mount_device("sd:", std::make_unique<driver::fat32::Device>());
		if (err != 0)
		{
			printf(ERROR_PREFIX_STRING "Failed to mount SD card entry to file system (%d). Retrying...\n", err);
			sleep(1);
			continue;
		}

		break;
	}

	platform_v1::sd::set_speed(25000);  // High speed mode
}

void unmount_sd_card()
{
	os::fs.unmount_device("sd:");
	driver::fat32::unmount_disk();
}

void init_sequence()
{
	platform_v1::uart1.set_config(platform_v1::frequency / BAUD_RATE, device::Uart::Parity::Odd, device::Uart::Stopbits::Bit1);

	mount_sd_card();
}

int main()
{
	Uart_mount_manager uart_mount_manager;

	init_sequence();

	while (true)
	{
		printf(ANSI_GREEN_COLOR ANSI_BOLD "bootloader" ANSI_RESET_COLOR ":$ ");
		fflush(stdout);

		char input[256] = {0};
		scanf("%s", input);

		printf("%s\n", input);

		if (input[0] == ':')
		{
			if (strcmp(input + 1, "ls") == 0)
				command_ls();
			else if (strcmp(input + 1, "info") == 0)
				command_info();
			else if (strcmp(input + 1, "exit") == 0)
				break;
			else
				printf(ERROR_PREFIX_STRING "Unknown command \"%s\"\n", input + 1);
		}
		else
		{
			const auto result = load_file(input);

			if (result.has_value())
			{
				unmount_sd_card();

				const auto [entry_address] = result.value();
				const auto [return_value, status] = ((std::pair<int, int>(*)(void))entry_address)();

				printf(ANSI_RESET_COLOR "\n");

				if (status != 0)
					printf(ERROR_PREFIX_STRING "Program exited abnormally with value %d\n", return_value);
				else
					printf("Program finished with return value %d\n", return_value);

				mount_sd_card();
			}
		}
	}

	return 0;
}