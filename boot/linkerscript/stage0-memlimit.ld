INCLUDE platform.ld
INCLUDE general_config.ld

MEMORY
{
	ROM (rx) : ORIGIN = ROM_BASE, LENGTH = ROM_SIZE
	RAM (rwx) : ORIGIN = RAM_BASE, LENGTH = MEM_LIMIT
}

SECTIONS
{
	. = ORIGIN(ROM);

	.text ALIGN(16): { 
		INCLUDE text.ld
	} > ROM

	.rodata ALIGN(16) : {
		INCLUDE rodata.ld
	} > ROM

	. = ORIGIN(RAM);

	.data ALIGN(16) : 
	{ 
		INCLUDE data.ld
	} > RAM AT>ROM

	.bss ALIGN(16) : 
	{ 
		INCLUDE bss.ld
	} > RAM

	INCLUDE stage0_const.ld
	
	PROVIDE(_heap_start = ALIGN(ADDR(.bss) + SIZEOF(.bss), 1024));
	PROVIDE(_heap_end = RAM_BASE + MEM_LIMIT);
	PROVIDE(_stack_top = RAM_BASE + RAM_SIZE);
}

ENTRY(_start)
