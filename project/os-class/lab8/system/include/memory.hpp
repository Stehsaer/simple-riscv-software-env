#pragma once

#include "env.hpp"

#include <array>
#include <cstddef>
#include <cstdint>

class Page_table_entry
{
	uint32_t value;

  public:

	Page_table_entry& operator=(const Page_table_entry&) = default;
	Page_table_entry& operator=(Page_table_entry&&) = default;
	Page_table_entry(const Page_table_entry&) = default;
	Page_table_entry(Page_table_entry&&) = default;

	struct Priviledge
	{
		bool v : 1 = false, r : 1 = false, w : 1 = false, x : 1 = false, u : 1 = false, g : 1 = false,
				 a : 1 = false, d : 1 = false;
	};
	static_assert(sizeof(Priviledge) == 1, "Priviledge must be 1 byte in size!");

	Page_table_entry() :
		value(0)
	{
	}

	Page_table_entry(uintptr_t address, Priviledge priv);

	Priviledge get_priv() const;
	uintptr_t get_address() const;
};

class Page_table
{
	alignas(4096) std::array<Page_table_entry, 1024> table;

  public:

	Page_table() = default;

	uintptr_t get_base_address() const;
	void set_as_primary_table() const;

	Page_table_entry& operator[](uint16_t offset);

	const Page_table_entry& operator[](uint16_t offset) const;
};

inline void check_address_4kb_aligned(uintptr_t virtual_address)
{
	KERNEL_ASSERT((virtual_address & 0xfff) == 0, "Address must be 4KiB aligned!");
}