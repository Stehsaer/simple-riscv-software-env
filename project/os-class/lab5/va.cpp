#include "filesystem.hpp"
#include <cstdarg>
#include <iostream>

static void init_environment()
{
	auto& uart = platform::uart;
	filesystem::fs.mount_device("uart:/", std::make_unique<filesystem::driver::Serial>(uart));
	freopen("uart:/", "w", stdout);
}

static void my_printf(const char* format, ...)
{
	va_list args;
	va_start(args, format);

	const char* ptr = format;
	while (true)
	{
		if (*ptr == 0) break;
		if (*ptr != '%')
		{
			std::cout << *ptr;
			ptr++;
			continue;
		}

		ptr++;

		switch (*ptr)
		{
		case 0:
			continue;
		case 'd':
		{
			int value = va_arg(args, int);
			std::cout << std::to_string(value);
			break;
		}
		case 's':
		{
			const char* str = va_arg(args, const char*);
			std::cout << str;
			break;
		}
		default:
			std::cout << *ptr;
			break;
		}

		ptr++;
	}
}

int main()
{
	init_environment();
	my_printf("Test %%d: %d, Test %%s: %s\n", 123, "Hello World");
	return 0;
}