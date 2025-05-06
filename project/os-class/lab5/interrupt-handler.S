.global interrupt_handler
.global other_interrupt

.text

interrupt_handler:

	csrrw x31, mscratch, x31 
	bnez x31, save_registers 
	csrrw x31, mscratch, x31

	j switch

save_registers:

	# Save registers
	sw x1,  0(x31)
	sw x2,  4(x31)
	sw x3,  8(x31)
	sw x4,  12(x31)
	sw x5,  16(x31)
	sw x6,  20(x31)
	sw x7,  24(x31)
	sw x8,  28(x31)
	sw x9,  32(x31)
	sw x10, 36(x31)
	sw x11, 40(x31)
	sw x12, 44(x31)
	sw x13, 48(x31)
	sw x14, 52(x31)
	sw x15, 56(x31)
	sw x16, 60(x31)
	sw x17, 64(x31)
	sw x18, 68(x31)
	sw x19, 72(x31)
	sw x20, 76(x31)
	sw x21, 80(x31)
	sw x22, 84(x31)
	sw x23, 88(x31)
	sw x24, 92(x31)
	sw x25, 96(x31)
	sw x26, 100(x31)
	sw x27, 104(x31)
	sw x28, 108(x31)
	sw x29, 112(x31)
	sw x30, 116(x31)
	
	mv t0, x31
	csrrw x31, mscratch, t0 # Restore mscratch
	sw x31, 120(t0) # Save last register

switch:

	csrr t0, mcause

	li t1, 0x80000007 # Machine Timer Interrupt
	beq t0, t1, mtimer_interrupt

	j other_interrupt

mtimer_interrupt:

	call timer_interrupt
	j end

other_interrupt:
	j .
	
end:

	csrr x31, mscratch
	bnez x31, restore_register

	j return

restore_register:

	lw x1,  0(x31)
	lw x2,  4(x31)
	lw x3,  8(x31)
	lw x4,  12(x31)
	lw x5,  16(x31)
	lw x6,  20(x31)
	lw x7,  24(x31)
	lw x8,  28(x31)
	lw x9,  32(x31)
	lw x10, 36(x31)
	lw x11, 40(x31)
	lw x12, 44(x31)
	lw x13, 48(x31)
	lw x14, 52(x31)
	lw x15, 56(x31)
	lw x16, 60(x31)
	lw x17, 64(x31)
	lw x18, 68(x31)
	lw x19, 72(x31)
	lw x20, 76(x31)
	lw x21, 80(x31)
	lw x22, 84(x31)
	lw x23, 88(x31)
	lw x24, 92(x31)
	lw x25, 96(x31)
	lw x26, 100(x31)
	lw x27, 104(x31)
	lw x28, 108(x31)
	lw x29, 112(x31)
	lw x30, 116(x31)
	lw x31, 120(x31)

return:

	mret