#pragma once

#include <cstddef>
#include <cstdint>
#include <cstdlib>

#include "cpp-guard.h"

namespace device
{
	struct SPI
	{
		alignas(4) uint16_t divider;
		alignas(4) bool cs;
		alignas(4) uint16_t len;

	  private:

		alignas(4) uint8_t padding1[0x400 - 0x00C];

	  public:

		alignas(4) uint8_t tx[0x800 - 0x400];
		alignas(4) uint8_t rx[0xC00 - 0x800];

	  private:

		alignas(4) uint8_t padding2[0x1000 - 0xC00];

	  public:

		inline void set_rate(uint32_t freq, uint32_t rate) volatile noexcept { divider = freq / rate; }
		inline void send_bytes(uint16_t len) volatile noexcept { this->len = len; }
		inline void assert_cs() volatile noexcept { cs = true; }
		inline void deassert_cs() volatile noexcept { cs = false; }
		inline bool done() const volatile noexcept { return len == 0; }
		inline void wait() const volatile noexcept
		{
			while (!done())
			{
			}
		}
	};
}