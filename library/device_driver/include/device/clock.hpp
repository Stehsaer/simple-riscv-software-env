#pragma once

#include <cstddef>
#include <cstdint>
#include <cstdlib>

#include "cpp-guard.h"

namespace device
{
	class Clock
	{
		uint32_t lo;
		uint32_t hi;
		uint32_t cmp_lo;
		uint32_t cmp_hi;

	  public:

		uint32_t get_tick32() const volatile noexcept;
		uint64_t get_tick64() const volatile noexcept;

		void set_tick(uint64_t tick) volatile noexcept;
		void set_cmp(uint64_t tick) volatile noexcept;
	};
}