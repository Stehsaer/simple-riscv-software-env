#pragma once

#include <cstddef>
#ifndef __cplusplus
#error "Don't include from C!"
#endif

struct Address_config
{
	void *const heap_start, *const max_heap_end, *const periph_start;
};

extern "C"
{
	extern const Address_config platform_config;
}

extern size_t stdin_block, stdout_block;