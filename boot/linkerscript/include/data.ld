PROVIDE( __global_pointer$ = . + 0x800 );
*(.sdata .sdata.* .gnu.linkonce.s.*)
*(.sdata2 .sdata2.* .gnu.linkonce.s2.*)
*(.data .data.* .gnu.linkonce.d.*)
*(.got)
*(.got.*)
		
