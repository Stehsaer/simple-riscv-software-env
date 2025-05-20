PROVIDE(_data_lma = LOADADDR(.data));
PROVIDE(_data_vma = ADDR(.data));
PROVIDE(_data_size = SIZEOF(.data));
PROVIDE(_bss_start = ADDR(.bss));
PROVIDE(_bss_size = SIZEOF(.bss));
