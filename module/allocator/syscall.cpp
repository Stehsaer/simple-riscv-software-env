#include <cerrno>
#include <cstddef>
#include <cstdint>
#include <sys/types.h>

extern "C"
{
	extern uint8_t _heap_start, _heap_end;
	static uint8_t* current_heap_end = nullptr;

	void* _sbrk(ptrdiff_t incr)
	{
		if (current_heap_end == nullptr) current_heap_end = &_heap_start;

		uint8_t* const prev_heap_end = current_heap_end;

		current_heap_end += incr;

		if (current_heap_end < &_heap_start || current_heap_end > &_heap_end)
		{
			errno = ENOMEM;
			return (caddr_t)-1;
		}

		return (caddr_t)prev_heap_end;
	}
}