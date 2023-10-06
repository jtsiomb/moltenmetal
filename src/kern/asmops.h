/*
pcboot - bootable PC demo/game kernel
Copyright (C) 2018-2023  John Tsiombikas <nuclear@member.fsf.org>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY, without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
#ifndef ASMOPS_H_
#define ASMOPS_H_

#include <inttypes.h>

#define enable_intr() asm volatile("sti")
#define disable_intr() asm volatile("cli")
#define halt_cpu() asm volatile("hlt")

#define CALLER_EIP(eip) \
	asm volatile( \
		"mov 4(%%ebp), %0\n\t" \
		: "=a" ((uint32_t)eip))

static inline uint8_t inp(uint16_t port)
{
	uint8_t res;
	asm volatile (
		"inb %1, %0\n\t"
		: "=a" (res)
		: "dN" (port));
	return res;
}

static inline uint16_t inpw(uint16_t port)
{
	uint16_t res;
	asm volatile (
		"inw %1, %0\n\t"
		: "=a" (res)
		: "dN" (port));
	return res;
}

static inline uint32_t inpd(uint16_t port)
{
	uint32_t res;
	asm volatile (
		"inl %1, %0\n\t"
		: "=a" (res)
		: "dN" (port));
	return res;
}

#define outp(port, src) \
	asm volatile( \
		"outb %0, %1\n\t" \
		:: "a" ((uint8_t)(src)), "dN" ((uint16_t)(port)))

#define outpw(port, src) \
	asm volatile( \
		"outw %0, %1\n\t" \
		:: "a" ((uint16_t)(src)), "dN" ((uint16_t)(port)))

#define outpd(port, src) \
	asm volatile( \
		"outl %0, %1\n\t" \
		:: "a" ((uint32_t)(src)), "dN" ((uint16_t)(port)))

/* delay for about 1us */
#define iodelay() outp(0x80, 0)

#define get_ebp(ebp) \
	asm volatile( \
		"mov %%ebp, %0\n\t" \
		: "=g"((uint32_t)(ebp)))

#endif	/* ASMOPS_H_ */
