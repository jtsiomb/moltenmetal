; Molten Metal - Tech demo for the COM32 DOS protected mode system
; Copyright (C) 2023  John Tsiombikas <nuclear@mutantstargoat.com>
;
; This program is free software: you can redistribute it and/or modify
; it under the terms of the GNU General Public License as published by
; the Free Software Foundation, either version 3 of the License, or
; (at your option) any later version.
; 
; This program is distributed in the hope that it will be useful,
; but WITHOUT ANY WARRANTY; without even the implied warranty of
; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
; GNU General Public License for more details.
; 
; You should have received a copy of the GNU General Public License
; along with this program.  If not, see <https://www.gnu.org/licenses/>.
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
