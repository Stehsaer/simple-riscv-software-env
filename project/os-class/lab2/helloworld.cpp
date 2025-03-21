#include "module/file/driver/uart.hpp"
#include "module/file/interface.hpp"
#include "module/platform/qemu.hpp"

int main()
{
	auto& uart = platform::qemu::uart;
	file::fs.mount_device("uart:/", std::make_unique<file::driver::Qemu_uart_driver>(uart));

	freopen("uart:/", "w", stdout);

	printf("Hello World!\n");

	return 0;
}