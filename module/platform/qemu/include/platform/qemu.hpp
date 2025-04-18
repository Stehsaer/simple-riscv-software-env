#include "device/qemu.hpp"

namespace platform::qemu
{
	extern device::qemu::Uart& uart;
	inline constexpr uint32_t mtime_rate = 10000000;  // 10MHz
}