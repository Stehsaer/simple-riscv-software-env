#include "platform/kintex7.hpp"

#include <cerrno>
#include <sys/_timeval.h>
#include <sys/times.h>

#define PERIPH_BASE (0x0001'0000)
#define PERIPH_BLOCK(type, block) (*reinterpret_cast<type*>((uint8_t*)PERIPH_BASE + 4096 * (block)))

namespace platform::kintex7
{
	device::kintex7::Uart& uart = PERIPH_BLOCK(device::kintex7::Uart, 0);
	device::kintex7::Clock& clock = PERIPH_BLOCK(device::kintex7::Clock, 1);
	device::kintex7::SPI& spi_1 = PERIPH_BLOCK(device::kintex7::SPI, 2);

	uint64_t get_us()
	{
		return clock.get_timer() * 1000 / frequency_khz;
	}
}

extern "C"
{
	unsigned int sleep(unsigned int __seconds)
	{
		const uint64_t ticks = __seconds * CLOCKS_PER_SEC;
		const uint64_t start = platform::kintex7::get_us();
		while (platform::kintex7::get_us() - start < ticks)
		{
		}
		return 0;
	}

	clock_t _times(struct tms* buf)
	{
		const auto time_us = platform::kintex7::get_us();

		if (buf != nullptr) [[likely]]
		{
			buf->tms_cstime = 0;
			buf->tms_cutime = 0;
			buf->tms_stime = 0;
			buf->tms_utime = time_us % 0xFFFFFFFF;
		}

		return time_us % 0xFFFFFFFF;
	}

	int _gettimeofday(struct timeval* time, void*)
	{
		const auto time_us = platform::kintex7::get_us();

		if (time == nullptr) [[unlikely]]
		{
			errno = EFAULT;
			return -1;
		}

		time->tv_sec = (time_us / 1000000) & 0xFFFFFFFF;
		time->tv_usec = time_us % 1000000;

		return 0;
	}

	int usleep(useconds_t usec)
	{
		const uint64_t ticks = usec * CLOCKS_PER_SEC / 1000000ull;
		const uint64_t start = platform::kintex7::get_us();
		while (platform::kintex7::get_us() - start < ticks)
		{
		}
		return 0;
	}
}