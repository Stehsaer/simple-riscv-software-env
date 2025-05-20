#include "platform.hpp"

#include <cerrno>
#include <sys/_timeval.h>
#include <sys/times.h>

#define PERIPH_BASE (0x0001'0000)
#define PERIPH_BLOCK(type, block) (*reinterpret_cast<type*>((uint8_t*)PERIPH_BASE + 4096 * (block)))

namespace platform
{
	device::Uart& uart = PERIPH_BLOCK(device::Uart, 0);
	device::Clock& clock = PERIPH_BLOCK(device::Clock, 1);
	device::SPI& spi_1 = PERIPH_BLOCK(device::SPI, 2);
}

extern "C"
{
	uint64_t csr_get_timer()
	{
		return platform::clock.get_timer();
	}

	uint64_t csr_set_timer(uint64_t value)
	{
		const auto old = platform::clock.get_timer();
		platform::clock.set_timer(value);
		return old;
	}

	uint64_t csr_get_timecmp()
	{
		return platform::clock.get_timecmp();
	}

	uint64_t csr_set_timecmp(uint64_t value)
	{
		const auto old = platform::clock.get_timecmp();
		platform::clock.set_timecmp(value);
		return old;
	}
}