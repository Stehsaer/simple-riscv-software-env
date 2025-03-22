#pragma once

#include <cstdint>

namespace device::qemu
{
	struct Uart
	{
		alignas(4) volatile char tx_rx;
	};
}
