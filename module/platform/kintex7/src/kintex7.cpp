#include "module/platform/kintex7.hpp"

#include <cerrno>
#include <sys/_timeval.h>
#include <sys/times.h>

#define PERIPH_BASE (0x0001'0000)
#define PERIPH_BLOCK(type, block) (*reinterpret_cast<type*>((uint8_t*)PERIPH_BASE + 4096 * (block)))

namespace platform::kintex7
{
	const uint32_t frequency_khz = 200000;  // 200MHz

	device::Fpga_uart& uart = PERIPH_BLOCK(device::Fpga_uart, 0);
	device::Clock& clock = PERIPH_BLOCK(device::Clock, 1);
	device::SPI& spi_1 = PERIPH_BLOCK(device::SPI, 2);
}

extern "C"
{
	static uint64_t get_us()
	{
		return platform::kintex7::clock.get_timer() / (platform::kintex7::frequency_khz / 1000);
	}

	clock_t _times(struct tms* buf)
	{
		const auto time_us = get_us();

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
		const auto time_us = get_us();

		if (time == nullptr) [[unlikely]]
		{
			errno = EFAULT;
			return -1;
		}

		time->tv_sec = (time_us / 1000000) & 0xFFFFFFFF;
		time->tv_usec = time_us % 1000000;

		return 0;
	}
}