#pragma once

#include "device/clock.hpp"
#include "device/spi.hpp"
#include "device/uart.hpp"

namespace platform_v1
{
	extern volatile device::Uart& uart1;
	extern volatile device::Clock& clock;
	extern volatile device::SPI& spi_sd;
	extern const uint64_t frequency;

	uint64_t get_us();
}

extern "C"
{
	clock_t _times(struct tms* buf);
	int _gettimeofday(struct timeval*, void*);
	unsigned int sleep(unsigned int);
}