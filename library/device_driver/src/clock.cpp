#include "device/clock.hpp"

namespace device
{
	uint32_t Clock::get_tick32() const volatile noexcept
	{
		return lo;
	}

	uint64_t Clock::get_tick64() const volatile noexcept
	{
		return (static_cast<uint64_t>(hi) << 32) | static_cast<uint64_t>(lo);
	}

	void Clock::set_tick(uint64_t tick) volatile noexcept
	{
		hi = tick >> 32;
		lo = tick;
	}

	void Clock::set_cmp(uint64_t tick) volatile noexcept
	{
		cmp_hi = tick >> 32;
		cmp_lo = tick;
	}
}