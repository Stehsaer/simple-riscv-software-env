INCLUDE platform.ld
INCLUDE general_config.ld

MEMORY
{
	RAM (rwx) : ORIGIN = RAM_BASE + MEM_LIMIT, LENGTH = RAM_SIZE + MEM_LIMIT
}

SECTIONS
{
	. = ORIGIN(RAM);

	.text ALIGN(16): 
	{ 
		INCLUDE text.ld
	} > RAM

	.rodata ALIGN(16) : 
	{
		INCLUDE rodata.ld
	} > RAM

	.data ALIGN(16) : 
	{ 
		INCLUDE data.ld
	} > RAM

	.bss ALIGN(16) : 
	{ 
		INCLUDE bss.ld
	} > RAM

	.tdata ALIGN(16) : 
	{ 
		INCLUDE tdata.ld
	} > RAM

	.tbss ALIGN(16) : 
	{ 
		INCLUDE tbss.ld
	} > RAM

	PROVIDE(_tbss_start = ADDR(.tbss));
	PROVIDE(_tbss_size = SIZEOF(.tbss));
	PROVIDE(_tdata_start = ADDR(.tdata));
	PROVIDE(_tdata_size = SIZEOF(.tdata));
	PROVIDE(_bss_start = ADDR(.bss));
	PROVIDE(_bss_size = SIZEOF(.bss));

	.heap ALIGN(1024) :
	{
		PROVIDE(_heap_start = .);
		PROVIDE(_heap_end = RAM_BASE + RAM_SIZE - STACK_SIZE);
	} > RAM
}

ENTRY(_start)
