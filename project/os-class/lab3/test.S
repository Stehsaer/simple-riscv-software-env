.section .text.startup

	li t0, 0x10000000
	la t1, text1

loop:
	lb t2, 0(t1)
	beqz t2, end
	sb t2, 0(t0)
	addi t1, t1, 1
	j loop

end:
	li a0, 0
	ret


.section .rodata

text1:
	.asciz "Hello, world!\n"