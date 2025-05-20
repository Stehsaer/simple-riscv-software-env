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

*(.gcc_except_table)
*(.gcc_except_table.*)

*(.gnu.linkonce.r.*)