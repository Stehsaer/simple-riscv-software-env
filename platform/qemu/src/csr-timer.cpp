#include "csr-timer.hpp"

#ifdef RVISA_ZICSR

static auto* const csr_timer = (volatile int64_t*)0x0200bff8;
static auto* const csr_timecmp = (volatile uint64_t*)0x02004000;

extern "C"
{
	uint64_t csr_get_timer()
	{
		return *csr_timer;
	}

	uint64_t csr_set_timer(uint64_t value)
	{
		uint64_t old = *csr_timer;
		*csr_timer = value;
		return old;
	}

	uint64_t csr_get_timecmp()
	{
		return *csr_timecmp;
	}

	uint64_t csr_set_timecmp(uint64_t value)
	{
		uint64_t old = *csr_timecmp;
		*csr_timecmp = value;
		return old;
	}
}

#endif