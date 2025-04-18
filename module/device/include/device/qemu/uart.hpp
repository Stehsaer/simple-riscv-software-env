#pragma once

#include <cstdint>

namespace device::qemu
{
	struct Uart
	{
		volatile char tx_rx;  // 0x00
		uint8_t _padding1[4];
		volatile uint8_t lsr;  // 0x05

		static constexpr uint8_t lsr_rx_ready = 1 << 0;
	};
}
