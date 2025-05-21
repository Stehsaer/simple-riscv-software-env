PROVIDE(_ctor_start = .);
KEEP(*(.preinit_array))
KEEP(*(SORT_BY_INIT_PRIORITY(.init_array.*)))
KEEP(*(.init_array))
PROVIDE(_ctor_end = .);

PROVIDE(_dtor_start = .);
KEEP(*(SORT_BY_INIT_PRIORITY(.fini_array.*)))
KEEP(*(.fini_array))
PROVIDE(_dtor_end = .);

*(.rodata)
*(.rodata.*) 

*(.srodata)
*(.srodata.*)

*(.gnu.linkonce.r.*)