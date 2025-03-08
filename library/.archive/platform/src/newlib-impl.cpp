#include "platform/newlib-impl.hpp"
#include "platform/address-config.hpp"
#include "platform/periph.hpp"

#include <cstring>
#include <iterator>

size_t stdin_block = 0, stdout_block = 0;
static char* current_heap_end = nullptr;

extern "C"
{
	caddr_t _sbrk(int incr)
	{
		if (current_heap_end == nullptr) current_heap_end = (char*)platform_config.heap_start;

		char* const prev_heap_end = current_heap_end;

		current_heap_end += incr;

		if (current_heap_end < (char*)platform_config.heap_start || current_heap_end > (char*)platform_config.max_heap_end)
		{
			errno = ENOMEM;
			return (caddr_t)-1;
		}

		return (caddr_t)prev_heap_end;
	}

	int _write(int file, char* ptr, int len)
	{
		auto& uart = periph::get_block<periph::Uart>(stdout_block);

		for (int i = 0; i < len; i++)
		{
			while (!uart.status.tx_a);

			if (*ptr == 0) break;

			uart.tx = *ptr;

			++ptr;
		}

		return len;
	}

	int _read(int file, char* ptr, int len)
	{
		const auto& uart = periph::get_block<periph::Uart>(stdin_block);

		for (size_t i = 0; i < len; i++)
		{
			while (!uart.status.rx_a);
			ptr[i] = uart.rx;
			if (ptr[i] == '\n' || ptr[i] == '\r') return i + 1;
		}

		return len;
	}

	int _close(int file)
	{
		errno = ENOTSUP;
		return -1;
	}

	int _fstat(int file, struct stat* st)
	{
		st->st_mode = S_IFCHR;
		return 0;
	}

	int _open(const char* name, int flags, int mode)
	{
		errno = ENOTSUP;
		return -1;
	}

	int _isatty(int file)
	{
		return 1;
	}

	int _lseek(int file, int ptr, int dir)
	{
		return 0;
	}

	int _kill(int pid, int sig)
	{
		errno = ENOTSUP;
		return -1;
	}

	int _getpid(void)
	{
		return 1;
	}

	int _unlink(const char* name)
	{
		errno = ENOTSUP;
		return -1;
	}
}
