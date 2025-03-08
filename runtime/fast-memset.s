.global fast_memset

.text

# fast_memset(dst, c, n)
# Fill n bytes at dst with c.
fast_memset:

align_dst:

	andi t0, a0, 3
	beqz t0, align_dst_end
	sb a1, 0(a0)
	addi a0, a0, 1
	addi a2, a2, -1
	j align_dst

align_dst_end:
	
	andi t0, a1, 0xFF
	# t0: fill value
	slli t1, t0, 8
	or t0, t0, t1
	slli t1, t0, 16
	or t0, t0, t1

	# t1: compare threshold
	li t1, 128

large_loop:
	blt a2, t1, large_loop_end

	sw t0, 0(a0)
	sw t0, 4(a0)
	sw t0, 8(a0)
	sw t0, 12(a0)
	sw t0, 16(a0)
	sw t0, 20(a0)
	sw t0, 24(a0)
	sw t0, 28(a0)
	sw t0, 32(a0)
	sw t0, 36(a0)
	sw t0, 40(a0)
	sw t0, 44(a0)
	sw t0, 48(a0)
	sw t0, 52(a0)
	sw t0, 56(a0)
	sw t0, 60(a0)
	sw t0, 64(a0)
	sw t0, 68(a0)
	sw t0, 72(a0)
	sw t0, 76(a0)
	sw t0, 80(a0)
	sw t0, 84(a0)
	sw t0, 88(a0)
	sw t0, 92(a0)
	sw t0, 96(a0)
	sw t0, 100(a0)
	sw t0, 104(a0)
	sw t0, 108(a0)
	sw t0, 112(a0)
	sw t0, 116(a0)
	sw t0, 120(a0)
	sw t0, 124(a0)
	addi a0, a0, 128
	sub a2, a2, t1	
	j large_loop
	
large_loop_end:

	li t1, 4

small_loop_start:
	blt a2, t1, small_loop_end

	sw t0, 0(a0)
	addi a0, a0, 4
	sub a2, a2, t1
	j small_loop_start

small_loop_end:

	li t1, 1

last_byte_start:
	blt a2, t1, last_byte_end

	sb t0, 0(a0)
	addi a0, a0, 1
	sub a2, a2, t1
	j last_byte_start

last_byte_end:

	ret
