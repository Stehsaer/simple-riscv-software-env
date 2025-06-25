#include "memory.hpp"
#include "allocator.hpp"
#include "register-op.hpp"
#include "trap.hpp"

Page_table_entry::Page_table_entry(uintptr_t address, Priviledge priv)
{
	check_address_4kb_aligned(address);
	const uint8_t priv_int = *reinterpret_cast<uint8_t*>(&priv);
	value = (address >> 2) | (priv_int << 0);
}

Page_table_entry::Priviledge Page_table_entry::get_priv() const
{
	return *reinterpret_cast<const Priviledge*>(&value);
}

uintptr_t Page_table_entry::get_address() const
{
	return ((value >> 10) & 0xfffff) << 12;
}

uintptr_t Page_table::get_base_address() const
{
	return uintptr_t(table.data());
}

void Page_table::set_as_primary_table() const
{
	set_satp(true, get_base_address());
}

Page_table_entry& Page_table::operator[](uint16_t offset)
{
	KERNEL_ASSERT(offset < 1024, "Offset must be < 1024, logic error!");
	return table[offset];
}

const Page_table_entry& Page_table::operator[](uint16_t offset) const
{
	KERNEL_ASSERT(offset < 1024, "Offset must be < 1024, logic error!");
	return table[offset];
}
