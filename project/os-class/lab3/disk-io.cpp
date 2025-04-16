#include <iostream>
#include <print>
#include <vector>

#include <file/driver/fatfs/backend/virtio.hpp>
#include <file/driver/uart.hpp>
#include <file/interface.hpp>
#include <platform/qemu.hpp>

using namespace device::qemu::virtio;

namespace file::driver::fatfs
{
	extern std::unique_ptr<FATFS> fs;
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
		printf("Virtio device not found\n");
		return 1;
	}

	std::println("Found Virtio device at 0x{:08X}", (size_t)virtio);

	file::driver::fatfs::media_interface = std::make_unique<file::driver::fatfs::backend::virtio::Media_interface>(*virtio);

	while (true)
	{
		const auto mount_disk_result = file::driver::fatfs::mount_disk();
		if (mount_disk_result.has_value())
		{
			std::println("Failed to mount disk: {}, retrying", (int)mount_disk_result.value());
			continue;
		}

		break;
	}

	const auto mount_result = file::fs.mount_device("virtio:/", std::make_unique<file::driver::fatfs::Device>());

	if (mount_result != 0)
	{
		std::println("Failed to mount Virtio device: {}", mount_result);
		return 1;
	}

	// open test.txt and print out the content
	FILE* file = fopen("virtio:/test.txt", "r");

	if (file == nullptr)
	{
		std::println("Failed to open file");
		std::println("Last Failure: {}", (int)file::driver::fatfs::last_failure);
		std::println("Error: {}", (int)errno);
		return 1;
	}

	fseek(file, 0, SEEK_END);
	size_t file_size = ftell(file);
	fseek(file, 0, SEEK_SET);

	std::string buffer(file_size, '\0');
	fread(buffer.data(), 1, file_size, file);

	std::println("File content: {}", buffer);

	fclose(file);

	return 0;
}
