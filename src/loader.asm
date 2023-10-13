	section .loader

	extern startup
	extern _ldr_main_start
	extern _main_start
	extern _main_size
	extern boot_mem_map
	extern boot_mem_map_size

%include 'macros.inc'

	[bits 16]
	global _start
_start:
	cli
	mov ax, cs
	mov ds, ax
	mov es, ax
	mov fs, ax	; this will store the original real mode segment
	; modify the return to real mode jump segment
	mov [.jmpcs16 + 3], ax

	; put the stack on the next segment, should be free 
	add ax, 1000h
	mov ss, ax
	mov ax, 0xfffe
	mov sp, ax

%ifdef BUG_WARNING
	call warning	; issue a warning and allow the user to abort
%endif

	; check for VM86 and abort
	mov eax, cr0
	test ax, 1
	jz .notvm86

	; Under DOS 7.0 or later GEMMIS doesn't work and hangs the computer.
	; Detect DOS version and abort with an error message if ver >= 7.
	mov ah, 30h
	int 21h
	cmp al, 7
	jae .vm86abort2

	; try to return to real mode
	mov si, str_gemmis
	call printstr

	xor ax, ax
	mov bx, ax
	mov si, ax
	mov es, ax
	mov ds, ax
	mov cx, ax
	mov dx, ax
	mov ax, 1605h
	mov di, 30ah	; pretend to be windows 3.1
	int 2fh
	test cx, cx
	jnz .vm86abort
	; we got a function in ds:si
	push cs
	push ds
	pop es	; es <- func seg
	pop ds	; ds <- cs
	mov word [vmswitch_seg], es
	mov word [vmswitch_off], si
	xor ax, ax	; return to real mode
	cli		; just make sure nothing enabled intr behind out back
	call far [vmswitch]
	jc .vm86abort

	; success
	mov ax, cs
	mov ds, ax
	mov es, ax
	mov si, msg_okstr
	call printstr
	jmp .notvm86

.vm86abort:
	push cs
	pop ds
	mov si, msg_failstr
	call printstr
.vm86abort2:
	mov si, str_errvm86
	call printstr
	jmp exit

.notvm86:
%ifdef CON_SERIAL
	call setup_serial
%endif
	call enable_a20
	call detect_memory

	; calculate GDT linear address
	xor eax, eax
	mov ax, cs
	shl eax, 4
	add eax, gdt
	mov [gdt_base], eax

	; set tmp segment bases to match the linear address of our current seg
	xor eax, eax
	mov ax, cs
	shl eax, 4
	mov word [gdt + 18h + 2], ax	; tmp pm code base
	mov word [gdt + 20h + 2], ax	; tmp pm data base
	mov word [gdt + 28h + 2], ax	; ret-to-realmode code
	shr eax, 16
	mov byte [gdt + 18h + 4], al
	mov byte [gdt + 20h + 4], al
	mov byte [gdt + 28h + 4], al

	mov si, str_enterpm
	call printstr

	; change video mode now, to avoid having to bring in all the int86 code
	mov ax, 13h
	int 10h

	cli	; paranoid
	lgdt [gdt_lim]

	mov eax, cr0
	or eax, 1
	mov cr0, eax

	jmp 18h:.pm

	[bits 32]
.pm:	mov ax, 20h	; tmp data selector
	mov ds, ax
	mov ax, 10h	; dest data selector
	mov es, ax

	; copy main program high
	cld
	mov esi, _ldr_main_start
	mov edi, _main_start
	mov ecx, _main_size + 3
	shr ecx, 2
	rep movsd

	; copy memory map
	mov eax, [mem_map_size]
	mov [es:boot_mem_map_size], eax
	mov esi, mem_map
	mov edi, boot_mem_map
	mov ecx, 32	; 128 bytes
	rep movsd

	mov ax, 10h
	mov ds, ax
	mov ss, ax
	mov esp, _main_start

	call 8:startup
	cli

	; return to real mode
	jmp 28h:.rm

	[bits 16]
.rm:	mov eax, cr0
	and ax, 0xfffe
	mov cr0, eax
.jmpcs16:
	jmp 42h:.loadcs16	; 42 seg is modifed at the start
.loadcs16:
	mov ax, fs
	mov ds, ax
	mov es, ax
	mov ss, ax

	; restore real-mode IVT
	o32 lidt [rmidt]

	; switch back to text mode
	mov ax, 3
	int 10h

	; if we used GEMMIS to exit vm86, switch back to it
	cmp dword [vmswitch], 0
	jz exit
	mov ax, 1
	call far [vmswitch]
	; broadcast windows exit
	xor ax, ax
	mov bx, ax
	mov si, ax
	mov es, ax
	mov ds, ax
	mov cx, ax
	mov dx, ax
	mov ax, 1606h
	int 2fh

exit:	mov ax, 4c00h
	int 21h

str_gemmis db 'Memory manager detected, trying to take control...',0
str_errvm86 db 'Error: memory manager running. Stop it and try again (e.g. emm386 off)',10,0
str_enterpm db 'Entering 32bit protected mode ...',10,0

	align 4
vmswitch:
vmswitch_off dw 0
vmswitch_seg dw 0

%ifdef BUG_WARNING
	; warns the user about the experimental and buggy state of this
	; protected mode system, and allows aborting if having to reboot later
	; is not acceptable.
warning:
	mov si, str_warnmsg
	call printstr
	WAITKEY
	dec al
	jz exit
	ret

str_warnmsg:
	db 'WARNING: this program uses an experimental protected mode kernel, which is ',10
	db 'still in a very early stage of development, and will probably not be able to ',10
	db 'return cleanly to DOS on exit. You might be forced to reboot after quitting!',10
	db 'If this is not acceptable, press ESC now to abort.',10,10
	db 'Press ESC to abort and return to DOS, any other key to continue...',10,10,0
%endif	; BUG_WARNING


; ---------------------- A20 address line enable -----------------------

enable_a20:
	call test_a20
	jnc .ret

	mov si, .infomsg
	call printstr		; print "Enable A20 line ... "

	call enable_a20_kbd
	call test_a20
	jnc .done
	call enable_a20_fast
	call test_a20
	jnc .done
	mov si, msg_failstr
	call printstr
	mov ax, 4c00h
	int 21h
.done:	mov si, msg_okstr
	call printstr
.ret:	ret

.infomsg db 'Enable A20 line:',0
msg_failstr db ' failed.',10,0
msg_okstr db ' success.',10,0

	; CF = 1 if A20 test fails (not enabled)
test_a20:
	push ds
	push es
	xor ax, ax
	mov ds, ax
	not ax
	mov es, ax
	mov si, 420h
	mov di, 430h
	mov dl, [ds:si]
	mov dh, [es:di]
	mov [ds:si], al
	not ax
	mov [es:di], al
	cmp al, [ds:si]
	clc
	jnz .done
	stc
.done:	mov [ds:si], dl
	mov [es:di], dh
	pop es
	pop ds
	ret

enable_a20_fast:
	mov si, .info
	call printstr
	in al, 92h
	or al, 2
	out 92h, al
	ret
.info db ' fast ...',0

KBC_DATA_PORT equ 0x60
KBC_CMD_PORT equ 0x64
KBC_STATUS_PORT equ 0x64
KBC_CMD_WR_OUTPORT equ 0xd1

KBC_STAT_IN_FULL equ 2

enable_a20_kbd:
	mov si, .info
	call printstr
	call kbc_wait_write
	mov al, KBC_CMD_WR_OUTPORT
	out KBC_CMD_PORT, al
	call kbc_wait_write
	mov al, 0xdf
	out KBC_DATA_PORT, al
	ret
.info db ' kbd ...',0

kbc_wait_write:
	in al, KBC_STATUS_PORT
	and al, KBC_STAT_IN_FULL
	jnz kbc_wait_write
	ret


; ---------------------- memory detection -----------------------

detect_memory:
	mov si, memdet_detram
	call printstr
	mov si, memdet_e820_msg
	call printstr
	call detect_mem_e820
	jnc .done
	mov si, str_fail
	call printstr

	mov si, memdet_detram
	call printstr
	mov si, memdet_e801_msg
	call printstr
	call detect_mem_e801
	jnc .done
	mov si, str_fail
	call printstr

	mov si, memdet_detram
	call printstr
	mov esi, memdet_88_msg
	call printstr
	call detect_mem_88
	jnc .done
	mov esi, str_fail
	call printstr

	mov si, memdet_detram
	call printstr
	mov esi, memdet_cmos_msg
	call printstr
	call detect_mem_cmos
	jnc .done
	mov esi, str_fail
	call printstr

	mov si, memdet_fail_msg
	call printstr
	jmp exit
.done:
	mov si, str_ok
	call printstr
	ret

str_ok db 'OK',10,0
str_fail db 'failed',10,0
memdet_fail_msg db 'Failed to detect available memory!',10,0
memdet_detram	db 'Detecting RAM ',0
memdet_e820_msg db '(BIOS 15h/0xe820)... ',0
memdet_e801_msg db '(BIOS 15h/0xe801)... ',0
memdet_88_msg	db '(BIOS 15h/0x88, max 64mb)... ',0
memdet_cmos_msg db '(CMOS)...',0

	; detect extended memory using BIOS call 15h/e820
detect_mem_e820:
	mov dword [mem_map_size], 0

	mov edi, .buffer
	xor ebx, ebx
	mov edx, 534d4150h

.looptop:
	mov eax, 0xe820
	mov ecx, 24
	int 15h
	jc .fail
	cmp eax, 534d4150h
	jnz .fail

	; skip areas starting above 4GB as we won't be able to use them
	cmp dword [di + 4], 0
	jnz .skip

	; only care for type 1 (usable ram), otherwise ignore
	cmp dword [di + 16], 1
	jnz .skip

	mov eax, [.buffer]
	mov esi, mem_map
	mov ebp, [mem_map_size]
	mov [ebp * 8 + esi], eax

	; skip areas with 0 size (also clamp size to 4gb)
	; test high 32bits
	cmp dword [edi + 12], 0
	jz .highzero
	; high part is non-zero, make low part ffffffff
	xor eax, eax
	not eax
	jmp .skiph0

.highzero:
	; if both high and low parts are zero, ignore
	mov eax, [di + 8]
	test eax, eax
	jz .skip

.skiph0:mov [ebp * 8 + esi + 4], eax
	inc dword [mem_map_size]

.skip:
	; terminate the loop if ebx was reset to 0
	test ebx, ebx
	jz .done
	jmp .looptop
.done:
	clc
	ret

.fail:	; if size > 0, then it's not a failure, just the end
	cmp dword [mem_map_size], 0
	jnz .done
	stc
	ret

.buffer times 32 db 0

	; detect extended memory using BIOS call 15h/e801
detect_mem_e801:
	mov si, mem_map
	mov ebp, [mem_map_size]
	mov dword [ebp], 0

	xor cx, cx
	xor dx, dx
	mov ax, 0xe801
	int 15h
	jc .fail

	test cx, cx
	jnz .foo1
	test ax, ax
	jz .fail
	mov cx, ax
	mov dx, bx

.foo1:	mov dword [si], 100000h
	movzx eax, cx
	; first size is in KB, convert to bytes
	shl eax, 10
	jnc .foo2
	; overflow means it's >4GB, clamp to 4GB
	mov eax, 0xffffffff
.foo2:	mov [si + 4], eax
	inc dword [mem_map_size]
	test dx, dx
	jz .done
	mov dword [si + 8], 1000000h
	movzx eax, dx
	; second size is in 64kb blocks, convert to bytes
	shl eax, 16
	jnc .foo3
	; overflow means it's >4GB, clamp to 4GB
	mov eax, 0xffffffff
.foo3:	mov [si + 12], eax
	inc dword [mem_map_size]
.done:
	clc
	ret
.fail:
	stc
	ret

detect_mem_88:
	; reportedly some BIOS implementations fail to clear CF on success
	clc
	mov ah, 88h
	int 15h
	jc .fail

	test ax, ax
	jz .fail

	; ax has size in KB, convert to bytes in eax
	and eax, 0xffff
	shl eax, 10

	mov esi, mem_map
	mov dword [si], 100000h
	mov [si + 4], eax

	mov dword [mem_map_size], 1
	clc
	ret
.fail:	stc
	ret

detect_mem_cmos:
	mov al, 31h
	out 70h, al
	in al, 71h
	mov ah, al
	mov al, 30h
	out 70h, al
	in al, 71h

	test ax, ax
	jz .fail

	; ax has size in KB, convert to bytes in eax
	and eax, 0xffff
	shl eax, 10

	mov esi, mem_map
	mov dword [si], 100000h
	mov [si + 4], eax
	mov dword [mem_map_size], 1
	clc
	ret
.fail:	stc
	ret


	align 4
mem_map_size dd 0
mem_map times 128 db 0


; ----------------------- serial console ------------------------

%ifdef CON_SERIAL
setup_serial:
	; set clock divisor
	mov dx, UART_BASE + 3	; LCTL
	mov al, 80h		; DLAB
	out dx, al
	mov dx, UART_BASE	; DIVLO
	mov al, UART_DIVISOR & 0xff
	out dx, al
	inc dx			; DIVHI
	mov al, UART_DIVISOR >> 8
	out dx, al
	mov dx, UART_BASE + 3	; LCTL
	mov al, 3		; 8n1
	out dx, al
	inc dx			; MCTL
	mov al, 0xb		; DTR/RTS/OUT2
	out dx, al
	ret

ser_putchar:
	SER_PUTCHAR
	ret

ser_printstr:
	lodsb
	test al, al
	jz .end
	cmp al, 10
	jnz .nolf
	push ax
	mov al, 13
	call ser_putchar
	pop ax
.nolf:	call ser_putchar
	jmp ser_printstr
.end:	ret

%endif ; def CON_SERIAL


printstr:
	lodsb
	test al, al
	jz .end
	cmp al, 10	; check for line-feed and insert CR before it
	jnz .nolf
	push ax
%ifdef CON_SERIAL
	mov al, 13
	call ser_putchar
%endif
	mov dl, 13
	mov ah, 2
	int 21h
	pop ax
.nolf:
%ifdef CON_SERIAL
	call ser_putchar
%endif
	mov ah, 2
	mov dl, al
	int 21h
	jmp printstr
.end:	ret


	align 4
enterpm dd 0xbad00d	; space for linear address for far jump to pmode
enterpm_sel dw 8	; selector for far jump to protected mode
	align 4
gdt_lim dw 47		; GDT limit
gdt_base dd 0xbadf00d	; space for GDT linear address

	align 8
gdt:	; 0: null segment
	dd 0
	dd 0
	; 1: code - 0/lim:4g, G:4k, 32bit, avl, pres|app, dpl:0, type:code/non-conf/rd (sel: 8)
	dd 0000ffffh
	dd 00cf9a00h
	; 2: data - 0/lim:4g, G:4k, 32bit, avl, pres|app, dpl:0, type:data/rw (sel: 10h)
	dd 0000ffffh
	dd 00cf9200h
	; 3: tmp code (will set base before entering pmode) (sel: 18h)
	dd 0000ffffh
	dd 00cf9a00h
	; 4: tmp data (will set base before entering pmode) (sel: 20h)
	dd 0000ffffh
	dd 00cf9200h
	; 5: return to real-mode 16bit code segment (sel: 28h)
	dd 0000ffffh
	dd 00009a00h

	; pseudo IDTR descriptor for real-mode IVT at address 0
	align 4
	dw 0
rmidt:	dw 3ffh		; IVT limit (1kb / 256 entries)
	dd 0		; IVT base 0


; --- debug ---
newline:
	push ax
	mov al, 13
	call ser_putchar
	mov al, 10
	call ser_putchar
	pop ax
	ret

printhex:
	rol ax, 4
	call print_hexdigit
	rol ax, 4
	call print_hexdigit
	rol ax, 4
	call print_hexdigit
	rol ax, 4
	call print_hexdigit
	ret

print_hexdigit:
	push ax
	and ax, 0xf
	cmp al, 0xa
	jae .hexdig
	add al, '0'
	call ser_putchar
	pop ax
	ret
.hexdig:add al, 'a' - 10
	call ser_putchar
	pop ax
	ret

; vi:set ts=8 sts=8 sw=8 ft=nasm:
