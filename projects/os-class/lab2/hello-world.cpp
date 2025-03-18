#include "os/env.hpp"
#include "os/file.hpp"
#include "qemu-devices/serial.hpp"

int main()
{
	os::fs.mount_device("serial:/", qemu_device::serial.get_device());
	freopen("serial:/", "r", stdin);
	freopen("serial:/", "w", stdout);

	printf("Hello, world!\n");

	return 0;
}