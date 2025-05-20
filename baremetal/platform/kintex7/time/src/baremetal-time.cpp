#include "baremetal-time.hpp"
#include "platform.hpp"

uint64_t platform_get_usecond()
{
	constexpr uint64_t ratio = device::Clock::frequency / 1'000'000;
	return platform::clock.get_timer() / ratio;
}