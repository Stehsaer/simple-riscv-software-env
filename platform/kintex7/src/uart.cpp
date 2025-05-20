#include "device/uart.hpp"

namespace device
{
	void Uart::configure(uint32_t divisor, Parity parity, Stopbits stopbits) noexcept
	{
		config = (divisor & 0x00FF'FFFF) | (static_cast<uint32_t>(parity) << 24)
			   | (static_cast<uint32_t>(stopbits) << 27);
	}
}