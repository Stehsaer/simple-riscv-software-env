#include "csr-timer.hpp"
#include "platform.hpp"

#include <sys/_timeval.h>
#include <sys/times.h>

namespace platform
{
	device::Uart& uart = *(device::Uart*)0x1000'0000;
}