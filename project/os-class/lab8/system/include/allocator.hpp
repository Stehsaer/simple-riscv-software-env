#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <ranges>
#include <vector>

namespace allocator
{
	uintptr_t allocate_page();
	void deallocate_page(uintptr_t address);

	uint32_t get_total_pages();
	uint32_t get_allocated_pages();

	void copy_page(uintptr_t from, uintptr_t to);
	void zero_page(uintptr_t address);
}