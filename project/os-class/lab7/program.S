.global code_start
.global code_end

.option push
.option pic

.section .rodata

.align 4
code_start:
	li t0, 0x20000000 # data
	li t1, 0x30000000 # uart

loop:
	lb t2, 0(t0) 		# load byte from data
	beqz t2, loop_end 	# if byte is zero, exit loop
	sb t2, 0(t1) 		# store byte to uart
	addi t0, t0, 1		# increment data pointer
	j loop

loop_end:
	ecall

code_end:
	nop

.option pop