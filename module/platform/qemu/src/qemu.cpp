#include "module/platform/qemu.hpp"

namespace platform::qemu
{
	device::Qemu_uart& uart = *(device::Qemu_uart*)0x1000'0000;
}