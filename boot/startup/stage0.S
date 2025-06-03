.global _start
.global _exit
.global __errno
.global __dso_handle

.section .text.startup

_start:
	la sp, _stack_top

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

	call memset

	fence.i

	# Execute CTOR

.ifndef STAGE0_MEMLIMIT
	la a0, _eh_frame_start
	call __register_frame
.endif

	la s0, _ctor_start

	cpp_ctor:
		la s1, _ctor_end
		beq s0, s1, cpp_ctor_end
		lw s2, 0(s0)
		jalr s2
		add s0, s0, 4
		j cpp_ctor

	cpp_ctor_end:

	li a0, 0 # argc = 0
	li a1, 0 # argv = (void*)0
	call main

	# Execute DTOR

	la s0, _dtor_start
	cpp_dtor:
		la s1, _dtor_end
		beq s0, s1, cpp_dtor_end
		lw s2, 0(s0)
		jalr s2
		add s0, s0, 4
		j cpp_dtor

	cpp_dtor_end:

	# Infinite Loop
	j .

_exit:
	j _exit # Halt indefinitely

.data

__dso_handle:
    .word 0
