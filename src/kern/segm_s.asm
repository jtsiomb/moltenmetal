; pcboot - bootable PC demo/game kernel
; Copyright (C) 2018-2023  John Tsiombikas <nuclear@member.fsf.org>
; 
; This program is free software: you can redistribute it and/or modify
; it under the terms of the GNU General Public License as published by
; the Free Software Foundation, either version 3 of the License, or
; (at your option) any later version.
; 
; This program is distributed in the hope that it will be useful,
; but WITHOUT ANY WARRANTY, without even the implied warranty of
; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
; GNU General Public License for more details.
; 
; You should have received a copy of the GNU General Public License
; along with this program.  If not, see <https://www.gnu.org/licenses/>.

	section .data
	align 4
; memory reserved for setup_selectors
off:	dd 0
segm:	dw 0
; memory reserved for set_gdt
lim:	dw 0
addr:	dd 0

	section .text
; setup_selectors(uint16_t code, uint16_t data)
; loads the requested selectors to all the selector registers
	global setup_selectors
setup_selectors:
	; set data selectors directly
	mov eax, [esp + 8]
	mov ss, ax
	mov es, ax
	mov ds, ax
	mov gs, ax
	;mov fs, ax	; XXX don't touch fs, we use it to store initial seg
	; set cs using a long jump
	mov eax, [esp + 4]
	mov [segm], ax
	mov dword [off], .ldcs
	jmp [off]
.ldcs:	ret

; set_gdt(uint32_t addr, uint16_t limit)
; loads the GDTR with the new address and limit for the GDT
	global set_gdt
set_gdt:
	mov eax, [esp + 4]
	mov [addr], eax
	mov ax, [esp + 8]
	mov [lim], ax
	lgdt [lim]
	ret

; set_task_reg(uint16_t tss_selector)
; loads the TSS selector in the task register
	global set_task_reg
set_task_reg:
	mov eax, [esp + 4]
	ltr [esp + 4]
	ret

; vi:set ts=8 sts=8 sw=8 ft=nasm:
