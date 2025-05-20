#pragma once

#include <cstdint>

namespace device
{
	class Clock
	{
	  private:

		alignas(4) volatile uint32_t timer_l;  // 0x00
		alignas(4) volatile uint32_t timer_h;  // 0x04
		alignas(4) uint32_t timecmp_l;         // 0x08
		alignas(4) uint32_t timecmp_h;         // 0x0C

	  public:

		uint64_t get_timecmp() const noexcept;
		void set_timecmp(uint64_t time) noexcept;

		uint64_t get_timer() const noexcept;
		void set_timer(uint64_t time) noexcept;

		static constexpr auto frequency = 200'000'000;  // 200MHz
	};
}
