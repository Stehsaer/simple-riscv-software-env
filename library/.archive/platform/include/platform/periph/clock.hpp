#pragma once

#ifndef __cplusplus
#error "Don't include from C!"
#endif

#include <cstddef>

#include "periph-base.hpp"

namespace periph
{
	class Clock
	{
		uint32_t lo;
		uint32_t hi;

	  public:

		uint32_t get_tick32() const volatile;
		uint64_t get_tick64() const volatile;

		void set_tick(uint64_t tick) volatile;
	};
}