.global _start
.global _exit
.global __errno
.global __dso_handle
.global platform_config

.section .text.startup

_start:
	la sp, _stack_top
	add sp, sp, -16

.option push
.option norelax
	lla gp, __global_pointer$
.option pop

	# Copy Data Section
	la a0, _data_vma
	la a1, _data_lma
	la a2, _data_size

	call memcpy

	# Fill BSS Section with 0
	la a0, _bss_start
	li a1, 0
	la a2, _bss_size

	call fast_memset

	fence.i

	# Execute CTOR

	la t0, _ctor_start

	cpp_ctor:
		la t1, _ctor_end
		beq t0, t1, cpp_ctor_end

		lw t2, 0(t0)

		sw t0, 4(sp)
		jalr t2
		lw t0, 4(sp)

		add t0, t0, 4
		j cpp_ctor

	cpp_ctor_end:

	li a0, 0 # argc = 0
	li a1, 0 # argv = (void*)0
	call main

	# Execute DTOR

	la t0, _dtor_start

	cpp_dtor:
		la t1, _dtor_end
		beq t0, t1, cpp_dtor_end

		lw t2, 0(t0)

		sw t0, 4(sp)
		jalr t2
		lw t0, 4(sp)

		add t0, t0, 4
		j cpp_dtor

	cpp_dtor_end:

	# Infinite Loop
	j .

_exit:
	j _exit # Halt indefinitely

.data

__dso_handle:
    .word 0

platform_config:
	.word _heap_start
	.word _max_heap_end
	.word _periph_start
