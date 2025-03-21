#pragma once

#include "module/device.hpp"

#include <cstdint>
#include <ctime>

namespace platform::kintex7
{
	extern const uint32_t frequency_khz;
	extern device::Fpga_uart& uart;
	extern device::Clock& clock;
	extern device::SPI& spi_1;
}

extern "C"
{
	clock_t _times(struct tms* buf);
	int _gettimeofday(struct timeval*, void*);
}