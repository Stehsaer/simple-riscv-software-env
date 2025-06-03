. = ALIGN(16);

PROVIDE(_eh_frame_start = .);
KEEP(*(.eh_frame .eh_frame.*) )
LONG(0x0)
LONG(0x0)
LONG(0x0)
LONG(0x0)
PROVIDE(_eh_frame_end = .);

*(.eh_frame_hdr)
*(.eh_frame_entry .eh_frame_entry.*)

*(.sframe) *(.sframe.*)
*(.gnu_extab .gnu_extab*)
*(.exception_ranges*)

KEEP(*(.gcc_except_table .gcc_except_table.*))
