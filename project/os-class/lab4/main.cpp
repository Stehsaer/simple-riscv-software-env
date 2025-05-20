#include <iostream>
#include <print>
#include <vector>

#include "filesystem.hpp"

extern "C"
{
	void timer_interrupt()
	{
		static int count = 0;
		count++;

		const auto prev_timecmp = csr_get_timecmp();
		printf("Timer interrupt, count=%d\n", count);
		csr_set_timecmp(prev_timecmp + platform::mtime_rate);
	}

	void init_interrupt();
}

int main()
{
	csr_set_timecmp(platform::mtime_rate);
	auto& uart = platform::uart;
	filesystem::fs.mount_device("uart:/", std::make_unique<filesystem::driver::Serial>(uart));

	freopen("uart:/", "w", stdout);

	printf("Enabling interrupt\n");
	init_interrupt();

	while (true)
	{
		__asm__ volatile("wfi");
	}

	return -1;
}
