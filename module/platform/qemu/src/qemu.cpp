#include "platform/qemu.hpp"
#include "device/csr-timer.hpp"

#include <sys/_timeval.h>
#include <sys/times.h>

namespace platform::qemu
{
	device::qemu::Uart& uart = *(device::qemu::Uart*)0x1000'0000;

	uint64_t get_us()
	{
		return csr_get_timer() / (mtime_rate / 1000000);
	}
}

extern "C"
{
	unsigned int sleep(unsigned int time)
	{
		const uint64_t ticks = time * CLOCKS_PER_SEC;
		const uint64_t start = platform::qemu::get_us();
		while (platform::qemu::get_us() - start < ticks)
		{
		}
		return 0;
	}

	clock_t _times(struct tms* buf)
	{
		const auto time_us = platform::qemu::get_us();

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
		const auto time_us = platform::qemu::get_us();

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