#pragma once

#include <cstdint>

// Works only with Zicsr enabled
#ifdef RVISA_ZICSR

extern "C"
{
	uint64_t csr_get_timer();
	uint64_t csr_set_timer(uint64_t value);

	uint64_t csr_get_timecmp();
	uint64_t csr_set_timecmp(uint64_t value);
}

#endif