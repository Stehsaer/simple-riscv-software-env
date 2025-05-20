#include "baremetal-time.hpp"
#include "csr-timer.hpp"
#include "platform.hpp"

uint64_t platform_get_usecond()
{
	constexpr uint64_t ratio = platform::mtime_rate / 1'000'000;
	return csr_get_timer() / ratio;
}