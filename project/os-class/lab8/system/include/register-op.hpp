#pragma once

#include <cstdint>

#include "env.hpp"

inline void set_satp(bool paging_enabled, uintptr_t page)
{
	if ((page & 0xfff) != 0) [[unlikely]]
		KERNEL_ERROR("Page table address must be 4KiB aligned!");

	const uint32_t satp_value = ((paging_enabled ? 0x1 : 0x0) << 31) | (page >> 12);
	asm volatile("csrw satp, %0" : : "r"(satp_value));
}

inline void setup_pmp(void)
{
	// Set up a PMP to permit access to all of memory.
	// Ignore the illegal-instruction trap if PMPs aren't supported.
	const uintptr_t pmpc = 0b11111;
	asm volatile("la t0, 1f\n\t"
				 "csrrw t0, mtvec, t0\n\t"
				 "csrw pmpaddr0, %1\n\t"
				 "csrw pmpcfg0, %0\n\t"
				 ".align 2\n\t"
				 "1: csrw mtvec, t0"
				 :
				 : "r"(pmpc), "r"(-1UL)
				 : "t0");
}

inline void set_mstatus_mpp_supervisor()
{
	uint32_t mstatus;
	asm volatile("csrr %0, mstatus" : "=r"(mstatus));
	mstatus &= ~(0b11 << 11);  // Clear MPP bits
	mstatus |= (0b01 << 11);   // Set MPP to 01 (Supervisor mode)
	asm volatile("csrw mstatus, %0" : : "r"(mstatus));
}
