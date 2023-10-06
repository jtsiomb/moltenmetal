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
	dw 0
idtr_desc:
lim:	dw 0
addr:	dd 0

	section .text
; void set_idt(uint32_t addr, uint16_t limit)
	global set_idt
set_idt:
	mov eax, [esp + 4]
	mov [addr], eax
	mov ax, [esp + 8]
	mov [lim], ax
	lidt [idtr_desc]
	ret

; int get_intr_flag()
	global get_intr_flag
get_intr_flag:
	pushf
	pop eax
	; bit 9 of eflags is IF
	shr eax, 9
	and eax, 1
	ret

; void set_intr_flag(int onoff)
	global set_intr_flag
set_intr_flag:
	cmp dword [esp + 4], 0
	jz .off
	sti
	ret
.off:	cli
	ret

; interrupt entry with error code macro
; this macro generates an interrupt entry point for the
; exceptions which include error codes in the stack frame
%macro ientry_err 2
	global intr_entry_%2
intr_entry_%2:
	push dword %1
	jmp intr_entry_common
%endmacro

; interrupt entry without error code macro
; this macro generates an interrupt entry point for the interrupts
; and exceptions which do not include error codes in the stack frame
; it pushes a dummy error code (0), to make the stack frame identical
%macro ientry_noerr 2
	global intr_entry_%2
intr_entry_%2:
	push dword 0
	push dword %1
	jmp intr_entry_common
%endmacro

; common code used by all entry points. calls dispatch_intr()
; defined in intr.c
	extern dispatch_intr
intr_entry_common:
	; save general purpose registers
	pusha
	call dispatch_intr
intr_ret_local:
	; restore general purpose registers
	popa
	; remove error code and intr num from stack
	add esp, 8
	iret

; special case for the timer interrupt, to avoid all the overhead of
; going through the C interrupt dispatcher 250 times each second
;	extern nticks
;	global intr_entry_fast_timer
;intr_entry_fast_timer:
;	inc dword [nticks]
;	; signal end of interrupt
;	push eax
;	mov al, 20h
;	out 20h, al
;	pop eax
;	iret

; special case for IRQ 7 and IRQ 15, to catch spurious interrupts
PIC1_CMD equ 0x20
PIC2_CMD equ 0xa0
OCW2_EOI equ 0x20
OCW3_ISR equ 0x0b

	global irq7_entry_check_spurious
irq7_entry_check_spurious:
	push eax
	mov al, OCW3_ISR
	out PIC1_CMD, al
	in al, PIC1_CMD
	and al, 80h
	pop eax
	jnz intr_entry_irq7
	iret

	global irq15_entry_check_spurious
irq15_entry_check_spurious:
	push eax
	mov al, OCW3_ISR
	out PIC2_CMD, al
	in al, PIC2_CMD
	and al, 80h
	jnz .legit
	; it was spurious, send EOI to master PIC and iret
	mov al, OCW2_EOI
	out PIC1_CMD, al
	pop eax
	iret
.legit:	pop eax
	jmp intr_entry_irq15


; XXX not necessary for now, just leaving it in in case it's useful
; down the road.
;
; intr_ret is called by context_switch to return from the kernel
; to userspace. The argument is a properly formed intr_frame
; structure with the saved context of the new task.
;
; First thing to do is remove the return address pointing back
; to context_switch, which then leaves us with a proper interrupt
; stack frame, so we can jump right in the middle of the regular
; interrupt return code above.
	global intr_ret
intr_ret:
	add esp, $4
	jmp intr_ret_local

; by including interrupts.h with ASM defined, the macros above
; are expanded to generate all required interrupt entry points
%define INTR_ENTRY_EC(n, name)		ientry_err n, name
%define INTR_ENTRY_NOEC(n, name)	ientry_noerr n, name

%include 'intrtab.h'

; vi:set ts=8 sts=8 sw=8 ft=nasm:
