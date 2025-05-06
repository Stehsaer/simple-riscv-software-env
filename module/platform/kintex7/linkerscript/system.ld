RAM_BASE = 0x80040000;
RAM_ALLOCATED_SIZE = 768K;

MEMORY
{
	STORAGE (rwx) : ORIGIN = RAM_BASE, LENGTH = RAM_ALLOCATED_SIZE
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
	PROVIDE(_tdata_start = ADDR(.tdata));
	PROVIDE(_tdata_size = SIZEOF(.tdata));

	.heap ALIGN(1024) :
	{
		PROVIDE(_heap_start = .);
		PROVIDE(_heap_end = ORIGIN(STORAGE) + RAM_ALLOCATED_SIZE);
	} > STORAGE
}

ENTRY(_start)
