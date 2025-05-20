.global init_interrupt

.text

init_interrupt:
	la t1, interrupt_handler
	andi t1, t1, 0xFFFFFFFC # mode=BASE
	csrrw zero, mtvec, t1

	la t1, 1
	sll t1, t1, 3 # mstatus.MIE
	csrrs zero, mstatus, t1

	la t1, 1
	sll t1, t1, 7 # mie.MTIE
	csrrs zero, mie, t1