; pcboot - bootable PC demo/game kernel
; Copyright (C) 2018  John Tsiombikas <nuclear@member.fsf.org>
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

	; standard C memset
	global memset
memset:
	push ebp
	mov ebp, esp
	push edi

	mov edi, [ebp + 8]
	push edi
	mov al, [ebp + 12]
	mov ah, al
	mov cx, ax
	shl eax, 16
	mov ax, cx
	mov ecx, [ebp + 16]

	test ecx, ecx
	jz .done

	; write 1, 2, or 3 times until we reache a 32bit-aligned dest address
	mov edx, edi
	and edx, 3
	jz .main
	jmp [.pre_tab + ecx * 4]

.pre_tab dd .main, .pre1, .pre2, .pre3
.pre3:	stosb
	dec ecx
.pre2:	stosb
	dec ecx
.pre1:	stosb
	dec ecx
	jz .done

	; edi is 32bit-aligned here, write ecx>>2 32bit values
.main:
	push ecx
	shr ecx, 2
	rep stosd
	pop ecx

	; write any trailing bytes
	and ecx, 3
	jmp [.post_tab + ecx * 4]

.post_tab dd .done, .post1, .post2, .post3
.post3:	stosb
.post2:	stosb
.post1:	stosb

.done:	pop eax
	pop edi
	pop ebp
	ret


	; same as memset but copies 16bit values, and the count is the number
	; of 16bit values to copy, not bytes.
	global memset16
memset16:
	push ebp
	mov ebp, esp
	push edi

	mov edi, [ebp + 8]
	push edi
	mov ax, [ebp + 12]
	shl eax, 16
	mov ax, [ebp + 12]
	mov ecx, [ebp + 16]

	test ecx, ecx
	jz .done

	mov edx, edi
	and edx, 3
	jz .main
	jmp [.pre_tab + edx * 4]

.pre_tab dd .main, .pre1, .pre2, .pre3
.pre3:
	; unaligned by +3:
	; same as next one, but jump to ms16main instead of falling through
	mov [edi], al
	mov [edi + ecx * 2 - 1], ah
	rol eax, 8
	inc edi
	dec ecx
	jz .done
	jmp .main
.pre1:
	; unaligned by +1:
	; - write low byte at edi
	; - write high byte at the end
	; - rotate by 8 for the rest
	; - decrement ecx
	mov [edi], al
	mov [edi + ecx * 2 - 1], ah
	rol eax, 8
	inc edi
	dec ecx
	jz .done
.pre2:
	; unaligned by +2
	stosw
	dec ecx
	jz .done

.main:
	push ecx
	shr ecx, 1
	rep stosd
	pop ecx

	and ecx, 1
	jz .done
	stosw
.done:
	pop eax
	pop edi
	pop ebp
	ret


	; standard C memcpy
	global memcpy
memcpy:
	push ebp
	mov ebp, esp
	push edi
	push esi

	mov edi, [ebp + 8]
	mov esi, [ebp + 12]
	mov ecx, [ebp + 16]

	test ecx, ecx
	jz .done

	mov edx, ecx
	shr ecx, 2
	rep movsd

	and edx, 3
	jmp [.post_tab + edx * 4]

.post_tab dd .done, .post1, .post2, .post3
.post3:	movsb
.post2:	movsb
.post1:	movsb

.done:
	pop esi
	pop edi
	pop ebp
	ret

; vi:set ts=8 sts=8 sw=8 ft=nasm:
