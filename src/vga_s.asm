	section .text
	bits 32

	global vga_setpal
vga_setpal:
	mov ecx, [esp + 8]
	test ecx, ecx
	jz .done
	push ebp
	mov ebp, esp
	push ebx
	mov dx, 3c8h
	mov eax, [ebp + 8]
	out dx, al
	inc dx
	mov ebx, [ebp + 16]
.loop:	mov al, [ebx]
	shr al, 2
	out dx, al
	mov al, [ebx + 1]
	shr al, 2
	out dx, al
	mov al, [ebx + 2]
	shr al, 2
	out dx, al
	add ebx, 3
	dec ecx
	jnz .loop
	pop ebx
	pop ebp
.done:	ret


	global vga_setpalent
vga_setpalent:
	mov dx, 3c8h
	mov eax, [esp + 4]
	out dx, al
	inc dx
	mov eax, [esp + 8]
	shr eax, 2
	out dx, al
	mov eax, [esp + 12]
	shr eax, 2
	out dx, al
	mov eax, [esp + 16]
	shr eax, 2
	out dx, al
	ret

; vi:set ts=8 sts=8 sw=8 ft=nasm:
