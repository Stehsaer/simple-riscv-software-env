#include "platform-v1/periph.hpp"

#include <sys/_timeval.h>
#include <sys/times.h>
#include <sys/timespec.h>

namespace platform_v1
{
	volatile device::Uart& uart1 = *(device::Uart*)0x0001'0000;
	volatile device::Clock& clock = *(device::Clock*)0x0001'1000;
	volatile device::SPI& spi_sd = *(device::SPI*)0x0001'2000;
	const uint64_t frequency = 200'000'000;  // Fixed 200MHz

	uint64_t get_ticks()
	{
		return clock.get_tick64();
	}
}

extern "C"
{
	clock_t _times(struct tms* buf)
	{
		const auto ticks = platform_v1::get_ticks() * CLOCKS_PER_SEC / platform_v1::frequency;

		if (buf != nullptr) [[likely]]
		{
			buf->tms_cstime = 0;
			buf->tms_cutime = 0;
			buf->tms_stime = 0;
			buf->tms_utime = ticks;
		}

		return ticks;
	}

	int _gettimeofday(struct timeval* tv, void* tz)
	{
		const auto ticks = platform_v1::get_ticks() * CLOCKS_PER_SEC / platform_v1::frequency;
		tv->tv_sec = ticks / CLOCKS_PER_SEC;
		tv->tv_usec = ticks % CLOCKS_PER_SEC;
		return 0;
	}

	unsigned int sleep(unsigned int time)
	{
		const uint64_t ticks = time * platform_v1::frequency;
		const uint64_t start = platform_v1::get_ticks();
		while (platform_v1::get_ticks() - start < ticks)
		{
		}
		return 0;
	}
}