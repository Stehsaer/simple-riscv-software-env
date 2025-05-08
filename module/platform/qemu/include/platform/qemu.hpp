#include "device/qemu.hpp"

#include <ctime>

namespace platform::qemu
{
	extern device::qemu::Uart& uart;
	inline constexpr uint32_t mtime_rate = 10000000;  // 10MHz

	uint64_t get_us();
}

extern "C"
{
	clock_t _times(struct tms* buf);
	int _gettimeofday(struct timeval*, void*);

	unsigned int sleep(unsigned int time);
}