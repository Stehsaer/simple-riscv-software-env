KEEP(*(.eh_frame .eh_frame.*) )
*(.eh_frame_hdr)
*(.eh_frame_entry .eh_frame_entry.*)

*(.sframe) *(.sframe.*)
*(.gnu_extab .gnu_extab*)
*(.exception_ranges*)

KEEP(*(.gcc_except_table .gcc_except_table.*))
