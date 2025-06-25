#include "env.hpp"
#include "filesystem.hpp"
#include "register-op.hpp"
#include "trap.hpp"
#include <cstdarg>

using namespace device::virtio;

extern "C" uintptr_t system_gp;

IO* virtio;

static IO& find_device()
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

	if (available_devices.empty()) KERNEL_ERROR("No Virtio devices found!");

	if (available_devices.size() > 1)
		KERNEL_ERROR("Multiple Virtio devices found!")
	else
		return available_devices[0];
}

bool serial_initialized = false;

void initialize_env()
{
	const auto mount_serial_result = filesystem::fs.mount_device(
		"serial:/",
		std::make_unique<filesystem::driver::Serial>(platform::uart)
	);
	if (mount_serial_result != 0) KERNEL_ERROR("Failed to mount serial device");

	freopen("serial:/", "w", stdout);
	freopen("serial:/", "w", stderr);
	freopen("serial:/", "r", stdin);
	serial_initialized = true;

	auto& fatfs_media_interface = filesystem::driver::fatfs::media_interface;
	IO& io = find_device();
	virtio = &io;
	fatfs_media_interface = std::make_unique<filesystem::driver::fatfs::backend::virtio::Media_interface>(io);

	const auto mount_disk_result = filesystem::driver::fatfs::mount_disk();
	if (mount_disk_result.has_value()) KERNEL_ERROR("Failed to mount disk device");

	const auto mount_disk_to_fs_result
		= filesystem::fs.mount_device("disk:/", std::make_unique<filesystem::driver::fatfs::Device>());
	if (mount_disk_to_fs_result != 0) KERNEL_ERROR("Failed to mount disk device");

	setup_pmp();
	set_trap_handler();

	asm volatile("mv %0, gp" : "=r"(system_gp));
}

void deinitialize_env()
{
	filesystem::fs.unmount_device("disk:/");
	filesystem::driver::fatfs::unmount_disk();
	filesystem::driver::fatfs::media_interface.reset();

	fclose(stdout);
	fclose(stderr);
	fclose(stdin);

	filesystem::fs.unmount_device("serial:/");
}

void printk(const char* format, ...)
{
	if (!serial_initialized) KERNEL_ERROR("Serial not initialized, cannot print!");

	printf("[KERNEL] ");
	va_list args;
	va_start(args, format);
	vprintf(format, args);
}

void print_raw(const char* str)
{
	for (const char* p = str; *p; ++p) platform::uart.tx_rx = *p;
}

void enable_timer_interrupt()
{
	const auto timer_now = csr_get_timer();
	csr_set_timecmp(timer_now + 10000);  // Set timer to trigger

	asm volatile("csrs mstatus, %0" : : "r"(1 << 3));  // Set MIE bit in mstatus
	asm volatile("csrs mie, %0" : : "r"(1 << 7));      // Set MTIE bit in mie
}
