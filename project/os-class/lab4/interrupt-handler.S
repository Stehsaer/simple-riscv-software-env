.global interrupt_handler

.text

interrupt_handler:
	addi sp, sp, -64

	# Save registers
	sw ra, 60(sp)
	sw t0, 56(sp)
	sw t1, 52(sp)
	sw t2, 48(sp)
	sw t3, 44(sp)
	sw t4, 40(sp)
	sw t5, 36(sp)
	sw t6, 32(sp)
	sw a0, 28(sp)
	sw a1, 24(sp)
	sw a2, 20(sp)
	sw a3, 16(sp)
	sw a4, 12(sp)
	sw a5, 8(sp)
	sw a6, 4(sp)
	sw a7, 0(sp)

	csrrw t0, mcause, zero # Read mcause

	li t1, 0x80000007 # Machine Timer Interrupt
	beq t0, t1, mtimer_interrupt

	j end

mtimer_interrupt:

	call timer_interrupt # Call the timer interrupt handler

	j end
	
end:
	# Restore registers
	lw ra, 60(sp)
	lw t0, 56(sp)
	lw t1, 52(sp)
	lw t2, 48(sp)
	lw t3, 44(sp)
	lw t4, 40(sp)
	lw t5, 36(sp)
	lw t6, 32(sp)
	lw a0, 28(sp)
	lw a1, 24(sp)
	lw a2, 20(sp)
	lw a3, 16(sp)
	lw a4, 12(sp)
	lw a5, 8(sp)
	lw a6, 4(sp)
	lw a7, 0(sp)

	addi sp, sp, 64

	mret