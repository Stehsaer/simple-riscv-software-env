#include "core_portme.h"

#include "os/env.hpp"
#include "platform-v1/periph.hpp"

extern "C"
{
	void cpp_portable_init()
	{
		os::fs.mount_device("uart1:", platform_v1::uart1.get_device());
		freopen("uart1:", "w", stdout);
	}

	CORETIMETYPE
	barebones_clock()
	{
		return clock();
	}
}