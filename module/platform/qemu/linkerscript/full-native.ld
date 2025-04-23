/* Edit the sections to match the platform */

FLASH_BASE = 0x20000000;
FLASH_LENGTH = 32M;

RAM_BASE = 0x80000000;
RAM_SIZE = 2048M;
MAX_DATA_LENGTH = 2000M;

MEMORY
{
	FLASH (rx) : ORIGIN = FLASH_BASE, LENGTH = FLASH_LENGTH
	DATA_RAM (rwx) : ORIGIN = RAM_BASE, LENGTH = MAX_DATA_LENGTH
}

SECTIONS
{
	. = ORIGIN(FLASH);

	.text ALIGN(16): { 
		KEEP(*(.text._start))
		KEEP(*(.text.startup))
		KEEP(*(.text.startup.*))
		*(.text)
		*(.text.*)
	} > FLASH

	.rodata ALIGN(16) : {
	
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

		*(.srodata)
		*(.srodata.*)

		*(.gcc_except_table)
		*(.gcc_except_table.*)

	} > FLASH

	. = ORIGIN(DATA_RAM);

	.data ALIGN(16) : 
	{ 
		PROVIDE( __global_pointer$ = . + 0x800 );
		*(.sdata)
		*(.sdata.*)
		*(.data)
		*(.data.*)
		*(.tdata)
		*(.tdata.*)
		*(.got)
		*(.got.*)

	} > DATA_RAM AT>FLASH

	PROVIDE(_data_lma = LOADADDR(.data));
	PROVIDE(_data_vma = ADDR(.data));
	PROVIDE(_data_size = SIZEOF(.data));

	.bss ALIGN(16) : 
	{ 
		*(.sbss)
		*(.sbss.*)
		*(.bss)
		*(.bss.*)
		*(.tbss)
		*(.tbss.*)
		*(COMMON)
		
	} > DATA_RAM

	PROVIDE(_bss_start = ADDR(.bss));
	PROVIDE(_bss_size = SIZEOF(.bss));
	
	PROVIDE(_heap_start = ALIGN(ADDR(.bss) + SIZEOF(.bss), 1024));
	PROVIDE(_heap_end = ORIGIN(DATA_RAM) + MAX_DATA_LENGTH);

	PROVIDE(_stack_top = ORIGIN(DATA_RAM) + RAM_SIZE);
}

ENTRY(_start)
