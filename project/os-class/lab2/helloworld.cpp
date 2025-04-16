#include "file/driver/uart.hpp"
#include "file/interface.hpp"
#include "platform/qemu.hpp"

int main()
{
	auto& uart = platform::qemu::uart;
	file::fs.mount_device("uart:/", std::make_unique<file::driver::qemu::Uart_driver>(uart));

	freopen("uart:/", "w", stdout);

	printf("Hello World!\n");

	return 0;
}