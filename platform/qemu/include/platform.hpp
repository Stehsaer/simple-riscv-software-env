#pragma once

#include "csr-timer.hpp"
#include "device/uart.hpp"
#include "device/virtio.hpp"

#include <ctime>

namespace platform
{
	extern device::Uart& uart;
	inline constexpr uint32_t mtime_rate = 10000000;  // 10MHz
}
