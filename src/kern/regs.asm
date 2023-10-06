	section .text
	align 4

	global get_regs
get_regs:
	push ebp
	mov ebp, esp

	push edx
	mov edx, [ebp + 8]

	mov [edx], eax
	mov [edx + 4], ebx
	mov [edx + 8], ecx
	
	; juggle edx
	mov eax, edx
	pop edx
	mov [eax + 12], edx
	push edx
	mov edx, eax
	
	; those two are pointless in a function
	mov [edx + 16], esp
	mov [edx + 20], ebp

	mov [edx + 24], esi
	mov [edx + 28], edx

	pushf
	pop eax
	mov [edx + 32], eax

	mov [edx + 36], cs
	mov [edx + 40], ss
	mov [edx + 44], ds
	mov [edx + 48], es
	mov [edx + 52], fs
	mov [edx + 56], gs

	push ebx
	mov ebx, cr0
	mov [edx + 60], ebx
	;mov ebx, cr1
	;mov [edx + 64], ebx
	mov ebx, cr2
	mov [edx + 68], ebx
	mov ebx, cr3
	mov [edx + 72], ebx
	pop ebx

	pop edx
	pop ebp
	ret


; vi:set ts=8 sts=8 sw=8 ft=nasm:
