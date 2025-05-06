#pragma once

#include "device/kintex7.hpp"

#include <cstdint>
#include <ctime>

#include <unistd.h>

namespace platform::kintex7
{
	inline constexpr uint32_t frequency_khz = 200000;
	extern device::kintex7::Uart& uart;
	extern device::kintex7::Clock& clock;
	extern device::kintex7::SPI& spi_1;

	uint64_t get_us();
}

extern "C"
{
	clock_t _times(struct tms* buf);
	int _gettimeofday(struct timeval*, void*);
	int usleep(useconds_t usec);
}