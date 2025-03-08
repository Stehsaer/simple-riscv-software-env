#include "platform/periph/clock.hpp"

namespace periph
{
	uint32_t Clock::get_tick32() const volatile
	{
		return lo;
	}

	uint64_t Clock::get_tick64() const volatile
	{
		return (static_cast<uint64_t>(hi) << 32) | static_cast<uint64_t>(lo);
	}

	void Clock::set_tick(uint64_t tick) volatile
	{
		union
		{
			struct
			{
				uint32_t u32l, u32h;
			} comb;

			uint64_t result;
		};

		result = tick;

		lo = comb.u32l;
		hi = comb.u32h;
	}
}