#pragma once

#include "csr-timer.hpp"
#include "device/clock.hpp"
#include "device/spi.hpp"
#include "device/uart.hpp"

#include <cstdint>
#include <ctime>

#include <unistd.h>

namespace platform
{
	extern device::Uart& uart;
	extern device::Clock& clock;
	extern device::SPI& spi_1;
}