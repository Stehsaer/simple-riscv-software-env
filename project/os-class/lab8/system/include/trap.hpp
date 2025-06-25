#pragma once

#include "env.hpp"

#include <cstdint>

enum class Exception_code
{
	Instruction_address_misaligned = 0,
	Instruction_access_fault = 1,
	Illegal_instruction = 2,
	Breakpoint = 3,
	Load_address_misaligned = 4,
	Load_access_fault = 5,
	Store_address_misaligned = 6,
	Store_access_fault = 7,
	Ecall_u = 8,
	Ecall_s = 9,
	Ecall_m = 11,
	Instruction_page_fault = 12,
	Load_page_fault = 13,
	Store_page_fault = 15
};

enum class Interrupt_code
{
	Software_interrupt_s = 1,
	Software_interrupt_m = 3,
	Timer_interrupt_s = 5,
	Timer_interrupt_m = 7,
	External_interrupt_s = 9,
	External_interrupt_m = 11
};

extern "C" void asm_trap_handler();
extern "C" void cpp_trap_handler(uintptr_t mcause, uintptr_t mtval);

inline void set_trap_handler()
{
	asm volatile("csrw mtvec, %0" : : "r"(uintptr_t(asm_trap_handler) & ~0b11));
}
