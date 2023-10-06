%define BUG_WARNING
%define CON_SERIAL

UART_BASE	equ 2f8h		; COM1: 3f8, COM2: 2f8
UART_DIVISOR	equ 115200 / 9600	; 9600 baud

%macro WAITKEY 0
%%waitkey:
	in al, 64h
	test al, 1
	jz %%waitkey
	in al, 60h
%endmacro

%macro SER_PUTCHAR 0
	push dx
	push ax
	mov dx, UART_BASE + 5	; LSTAT
%%wait:	in al, dx
	test al, 20h		; TREG_EMPTY
	jz %%wait
	mov dx, UART_BASE
	pop ax
	out dx, al
	pop dx
%endmacro

%macro PMPRINT 1
	mov al, %1
	SER_PUTCHAR
%endmacro

; vi:set ts=8 sts=8 sw=8 ft=nasm:
