#pragma once

#include <cstdint>

namespace device
{
	struct Uart
	{
		alignas(4) volatile char tx;          // [Register] Transmit, @0x00
		alignas(4) volatile char rx;          // [Register] Receive, @0x04
		alignas(4) uint32_t config;           // [Register] Configuration, @0x08
		alignas(4) volatile uint32_t status;  // [Register] Status, @0x0C

		enum class Stopbits
		{
			Bit1 = 0,
			Bit2 = 1
		};

		enum class Parity
		{
			None = 0b000,
			Odd = 0b001,
			Even = 0b010,
			Always1 = 0b100,
			Always0 = 0b010
		};

		bool rx_available() const noexcept { return status & 0b1; }
		bool tx_available() const noexcept { return status & 0b10; }
		bool error() const noexcept { return status & 0b100; }
		void configure(uint32_t divisor, Parity parity, Stopbits stopbits) noexcept;
	};
}
