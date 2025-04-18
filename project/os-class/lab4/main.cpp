#include <iostream>
#include <print>
#include <vector>

#include <device/csr-timer.hpp>
#include <file/driver/uart.hpp>
#include <file/interface.hpp>
#include <platform/qemu.hpp>

extern "C"
{
	void timer_interrupt()
	{
		static int count = 0;
		count++;

		const auto prev_timecmp = csr_get_timecmp();
		printf("Timer interrupt, count=%d\n", count);
		csr_set_timecmp(prev_timecmp + platform::qemu::mtime_rate);
	}

	void init_interrupt();
}

int main()
{
	csr_set_timecmp(platform::qemu::mtime_rate);
	auto& uart = platform::qemu::uart;
	file::fs.mount_device("uart:/", std::make_unique<file::driver::qemu::Uart_driver>(uart));

	freopen("uart:/", "w", stdout);

	printf("Enabling interrupt\n");
	init_interrupt();

	while (true)
	{
		__asm__ volatile("wfi");
	}

	return -1;
}
