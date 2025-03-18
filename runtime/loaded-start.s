.global _start
.global _exit
.global __errno
.global __dso_handle
.global platform_config

.section .text.startup

_start:
	# Stack protection
	add sp, sp, -32
	sw ra, 8(sp)
	sw gp, 12(sp)
	sw tp, 16(sp)
	sw a0, 20(sp)
	sw a1, 24(sp)

	# Initialize thread pointer
	la tp, _thread_pointer_init

	# Initialize global pointer
.option push
.option norelax
	lla gp, __global_pointer$
.option pop

	# Store frame pointer for error handling
	la t0, frame_pointer_save
	sw sp, 0(t0)

	# Clear TBSS

	la a0, _tbss_start
	la a1, 0
	la a2, _tbss_size

	call memset

	# Clear BSS

	la a0, _bss_start
	li a1, 0
	la a2, _bss_size

	call memset

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

	# Call main
	lw a0, 20(sp)
	lw a1, 24(sp)
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

	# Stack restoring
	lw ra, 8(sp)
	lw gp, 12(sp)
	sw tp, 16(sp)
	add sp, sp, 32

	li a1, 0 # Exit successfully

	ret

abnormal_exit:

	la t0, frame_pointer_save
	lw sp, 0(t0) 

	mv a2, ra # Acquire exit address

	lw ra, 8(sp)
	lw gp, 12(sp)
	sw tp, 16(sp)
	add sp, sp, 32

	li a1, 1 # Exit with error

	ret

_exit:
	j abnormal_exit

.data
	.align 4

__dso_handle:
    .word 0

frame_pointer_save:
	.word 0

.section .rodata
	.align 4
	
platform_config:
	.word _heap_start
	.word _max_heap_end
	.word _periph_start

