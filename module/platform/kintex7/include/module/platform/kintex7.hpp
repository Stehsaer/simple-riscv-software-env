#pragma once

#include "module/device.hpp"

#include <cstdint>
#include <ctime>

namespace platform::kintex7
{
	extern const uint32_t frequency_khz;
	extern device::kintex7::Uart& uart;
	extern device::kintex7::Clock& clock;
	extern device::kintex7::SPI& spi_1;
}

extern "C"
{
	clock_t _times(struct tms* buf);
	int _gettimeofday(struct timeval*, void*);
}