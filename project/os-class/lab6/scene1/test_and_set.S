.global test_and_set

.section .text

# bool test_and_set(void* dst)
test_and_set:
	lr.w t0, (a0)
	bnez t0, fail
	li t0, 1
	sc.w t1, t0, (a0)
	bnez t1, fail
	fence rw, rw
	li a0, 1
	ret

fail:
	li a0, 0
	ret
