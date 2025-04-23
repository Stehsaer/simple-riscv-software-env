#include <cerrno>
#include <cstddef>
#include <cstdint>
#include <sys/types.h>

#ifdef RVISA_A
#include <stdatomic.h>
namespace
{
	std::atomic_intptr_t current_heap_end = 0;

	bool modify_heap_end(intptr_t original, intptr_t new_value)
	{
		return std::atomic_compare_exchange_strong(&current_heap_end, &original, new_value);
	}

	intptr_t get_heap_end()
	{
		return std::atomic_load(&current_heap_end);
	}
}
#else
namespace
{
	uint8_t* current_heap_end = nullptr;
}
#endif

extern "C"
{
	extern uint8_t _heap_start, _heap_end;

#ifdef RVISA_A
	void* _sbrk(ptrdiff_t incr)
	{
		while (true)
		{
			modify_heap_end(0, (intptr_t)&_heap_start);

			auto original_heap_end = get_heap_end();
			original_heap_end += incr;

			if (original_heap_end < (intptr_t)&_heap_start || original_heap_end > (intptr_t)&_heap_end)
			{
				errno = ENOMEM;
				return (caddr_t)-1;
			}

			if (modify_heap_end(original_heap_end - incr, original_heap_end))
				return (caddr_t)(original_heap_end - incr);
		}
	}
#else
	void* _sbrk(ptrdiff_t incr)  // Not reentrable, not thread-safe
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

#endif
}