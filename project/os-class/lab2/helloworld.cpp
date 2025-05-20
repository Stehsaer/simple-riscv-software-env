#include "filesystem.hpp"
#include "platform.hpp"

int main()
{
	auto& uart = platform::uart;
	filesystem::fs.mount_device("uart:/", std::make_unique<filesystem::driver::Serial>(uart));

	freopen("uart:/", "w", stdout);

	printf("Hello World!\n");

	return 0;
}