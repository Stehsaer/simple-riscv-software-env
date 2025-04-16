#include "platform/qemu.hpp"

namespace platform::qemu
{
	device::qemu::Uart& uart = *(device::qemu::Uart*)0x1000'0000;
}