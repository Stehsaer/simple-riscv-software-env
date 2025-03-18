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
#include <random>
#include <sys/unistd.h>

#include <print>
#include <vector>

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

int main(int argc, char** argv)
{
	Uart_mount_manager uart_mount_manager;

	printf("argc: %d\n", argc);
	for (int i = 0; i < argc; i++)
	{
		printf("argv[%d]: %s\n", i, argv[i]);
	}

	printf("Mounting SD card...\n");
	driver::sd::mount_media_handler();

	while (true)
	{
		platform_v1::sd::set_speed(400);
		const auto mount_result = driver::fat32::mount_disk();

		if (mount_result.has_value())
		{
			printf("Failed to mount SD card, error(%d)...\n", mount_result.value());
			sleep(1);
			continue;
		}

		printf("SD card mounted successfully\n");

		const auto err = os::fs.mount_device("sd:", std::make_unique<driver::fat32::Device>());
		if (err != 0)
		{
			printf("Failed to attach SD to filesystem, error(%d)...\n", err);
			sleep(1);
			continue;
		}

		printf("Mount successful at %s\n", "sd:");

		break;
	}

	platform_v1::sd::set_speed(20000);

	std::vector<uint8_t> test_data(1024 * 1024 * 8);
	std::vector<uint8_t> read_data(1024 * 1024 * 8);

	// Random generate using <random>
	printf("Generating data...\n");
	for (size_t i = 0; i < test_data.size(); i++)
	{
		test_data[i] = (i * 13106527) % 256;
	}

	printf("Writing data to file...\n");
	printf("Time stamp at start: %lu\n", clock());
	const auto write_start = clock();
	{
		FILE* write_file = fopen("sd:/test.bin", "w");
		if (write_file == nullptr)
		{
			printf("Failed to create file\n");
			return 1;
		}

		const auto written = fwrite(test_data.data(), 1, test_data.size(), write_file);

		if (written != test_data.size())
		{
			printf("Failed to write file\n");

			const auto query_state = driver::sd::query_sd_state();
			if (query_state.has_value())
			{
				printf("SD card state: %04X\n", query_state.value());
			}

			printf("SD card error: %d\n", (int)driver::sd::err);

			return 1;
		}

		fclose(write_file);
	}
	printf("Time stamp at end: %lu\n", clock());
	printf("Time taken: %lu ms\n", (clock() - write_start) / 1000);

	printf("Reading from file...\n");
	printf("Time stamp at start: %lu\n", clock());
	const auto read_start = clock();
	{
		FILE* read_file = fopen("sd:/test.bin", "r");
		if (read_file == nullptr)
		{
			printf("Failed to open file\n");
			return 1;
		}

		const auto read = fread(read_data.data(), 1, read_data.size(), read_file);

		if (read != test_data.size())
		{
			printf("Failed to read file\n");
			return 1;
		}

		fclose(read_file);
	}
	printf("Time stamp at end: %lu\n", clock());
	printf("Time taken: %lu ms\n", (clock() - read_start) / 1000);

	printf("Compare data...\n");

	for (size_t i = 0; i < test_data.size(); i++)
	{
		if (test_data[i] != read_data[i])
		{
			const auto min_idx = std::max(0, (int)i - 10);
			const auto max_idx = std::min<int>((int)test_data.size(), i + 10);

			printf("Expected: ");
			for (int j = min_idx; j < max_idx; j++)
			{
				printf("%02X ", test_data[j]);
			}
			printf("\n");

			printf("Actual:   ");
			for (int j = min_idx; j < max_idx; j++)
			{
				printf("%02X ", read_data[j]);
			}
			printf("\n");

			return 1;
		}
	}

	printf("Data match\n");

	return 0;
}