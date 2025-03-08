#include "os/env.hpp"
#include "platform-v1/periph.hpp"

extern "C"
{
	int dhrystone_main(int argc, const char* argv[]);
}

int main()
{
	os::fs.mount_device("uart1:", platform_v1::uart1.get_device());
	freopen("uart1:", "w", stdout);

	printf("Dhrystone Benchmark\n");

	const char* argv[] = {"dhrystone", "10000000"};
	dhrystone_main(2, argv);
	return 0;
}