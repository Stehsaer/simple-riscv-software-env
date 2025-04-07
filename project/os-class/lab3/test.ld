/* Edit the sections to match the platform */

RAM_BASE = 0x80100000;
RAM_SIZE = 2047M;
STACK_SIZE = 2M;

PERIPH_BASE = 0x00010000;

MEMORY
{
	STORAGE (rwx) : ORIGIN = RAM_BASE, LENGTH = RAM_SIZE
}

SECTIONS
{
	. = ORIGIN(STORAGE);

	.text ALIGN(16): 
	{ 
		KEEP(*(.text._start))
		KEEP(*(.text.startup))
		KEEP(*(.text.startup.*))
		*(.text)
		*(.text.*)

		*(.gcc_except_table)
		*(.gcc_except_table.*)

		*(.gnu.linkonce.t.*)

	} > STORAGE

	.rodata ALIGN(16) : 
	{
		*(.gnu.linkonce.r.*)

		PROVIDE(_ctor_start = .);
		KEEP(*(.init_array))
		KEEP(*(.init_array.*))
		PROVIDE(_ctor_end = .);

		PROVIDE(_dtor_start = .);
		KEEP(*(.fini_array))
		KEEP(*(.fini_array.*))
		PROVIDE(_dtor_end = .);

		*(.rodata)
		*(.rodata.*) 

		*(.eh_frame) 
		*(.eh_frame_hdr)

		*(.srodata)
		*(.srodata.*)

	} > STORAGE

	.data ALIGN(16) : 
	{ 
		PROVIDE( __global_pointer$ = . + 0x800 );

		*(.sdata .sdata.* .gnu.linkonce.s.*)
		*(.sdata2 .sdata2.* .gnu.linkonce.s2.*)
		*(.data .data.* .gnu.linkonce.d.*)

	} > STORAGE

	.bss ALIGN(16) : 
	{ 
		*(.sbss .sbss.* .gnu.linkonce.sb.* .scommon)
		*(.bss .bss.*)
		*(COMMON)

	} > STORAGE

	PROVIDE(_bss_start = ADDR(.bss));
	PROVIDE(_bss_size = SIZEOF(.bss));

	.tdata ALIGN(16) : 
	{ 
		*(.tdata)
		*(.tdata.*)

	} > STORAGE

	.tbss ALIGN(16) : 
	{ 
		*(.tbss)
		*(.tbss.*)

	} > STORAGE

	PROVIDE(_tbss_start = ADDR(.tbss));
	PROVIDE(_tbss_size = SIZEOF(.tbss));
	PROVIDE(_thread_pointer_init = ADDR(.tdata));

	.heap ALIGN(1024) :
	{
		PROVIDE(_heap_start = .);
		PROVIDE(_heap_end = ORIGIN(STORAGE) + LENGTH(STORAGE) - STACK_SIZE);
	} > STORAGE
	
	PROVIDE(_periph_start = PERIPH_BASE);
}

ENTRY(_start)
