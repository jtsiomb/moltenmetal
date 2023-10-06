OUTPUT_FORMAT(binary)
ENTRY(_start)

SECTIONS {
	/* loader starts at <seg>:100h */
	. = 0x100;
	.loader : { * (.loader); }

	. = ALIGN(4);
	_ldr_main_start = .;
	_ldr_main_end = _ldr_main_start + _main_size;

	/* main program will be moved to 2MB by the loader */
	. = 2M;
	.main ALIGN(4): AT (_ldr_main_start) {
		_main_start = .;
		* (.startup);
		* (.text*);
		* (.rodata*);
		* (.data*);
		. = ALIGN(4);
	}

	.bss ALIGN(4): AT (_ldr_main_start + SIZEOF(.main)) {
		. = ALIGN(4);
		_bss_start = .;
		* (.bss*);
		* (COMMON);
		. = ALIGN(4);
		_bss_end = .;
	}

	. = ALIGN(4);
	_main_size = . - _main_start;
	_mem_start = .;

	/DISCARD/ : {
		*(.note.GNU-stack);
		*(.comment);
	}
}
