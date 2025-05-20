#include "baremetal-time.hpp"

#include <cerrno>
#include <ctime>
#include <sys/_timeval.h>
#include <sys/times.h>

extern "C"
{
	unsigned int sleep(unsigned int __seconds)
	{
		const uint64_t ticks = __seconds * CLOCKS_PER_SEC;
		const uint64_t start = platform_get_usecond();
		while (platform_get_usecond() - start < ticks)
		{
		}
		return 0;
	}

	clock_t _times(struct tms* buf)
	{
		const auto time_us = platform_get_usecond();

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
		const auto time_us = platform_get_usecond();

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
		const uint64_t start = platform_get_usecond();
		while (platform_get_usecond() - start < ticks)
		{
		}
		return 0;
	}
}