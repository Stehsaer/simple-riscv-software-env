#include "module/device/kintex7/clock.hpp"

namespace device::kintex7
{
	[[gnu::noinline]] uint64_t Clock::get_timecmp() const noexcept
	{
		return (static_cast<uint64_t>(timecmp_h) << 32) | timecmp_l;
	}

	[[gnu::noinline]] void Clock::set_timecmp(uint64_t time) noexcept
	{
		timecmp_l = time & 0xFFFFFFFF;
		timecmp_h = time >> 32;
	}

	[[gnu::noinline]] uint64_t Clock::get_timer() const noexcept
	{
		const uint32_t timer_h = this->timer_h;
		const uint32_t timer_l = this->timer_l;

		return (static_cast<uint64_t>(timer_h) << 32) | timer_l;
	}

	[[gnu::noinline]] void Clock::set_timer(uint64_t time) noexcept
	{
		timer_l = 0;
		timer_h = time >> 32;
		timer_l = time & 0xFFFFFFFF;
	}
}