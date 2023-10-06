	bits 32
	section .text

%include 'macros.inc'

	extern main
	extern _bss_start
	extern _bss_end

	global startup
startup:
	; clear .bss
	mov eax, _bss_end
	sub eax, _bss_start
	test eax, eax
	jz .nobss
	mov ecx, eax
	mov edi, _bss_start
	xor eax, eax
	shr ecx, 2
	rep stosd
.nobss:

	xor ebp, ebp	; terminate backtraces
	call main
	retf

;	global putchar
;putchar:
;	mov eax, [esp + 4]
;	cmp al, 10
;	jnz .nonl
;	push eax
;	mov al, 13
;	SER_PUTCHAR
;	pop eax
;.nonl:	SER_PUTCHAR
;	ret

	section .data
	global boot_mem_map_size
	global boot_mem_map
	align 4
boot_mem_map_size dd 0
boot_mem_map times 128 db 0

; vi:set ts=8 sts=8 sw=8 ft=nasm:
