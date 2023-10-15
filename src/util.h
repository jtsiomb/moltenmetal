/* Molten Metal - Tech demo for the COM32 DOS protected mode system
 * Copyright (C) 2023  John Tsiombikas <nuclear@mutantstargoat.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef UTIL_H_
#define UTIL_H_

#include <stdlib.h>
#include "inttypes.h"

/* fast conversion of double -> 32bit int
 * for details see:
 *  - http://chrishecker.com/images/f/fb/Gdmfp.pdf
 *  - http://stereopsis.com/FPU.html#convert
 */
static inline int32_t cround64(double val)
{
	val += 6755399441055744.0;
	return *(int32_t*)&val;
}

static inline float rsqrt(float x)
{
	float xhalf = x * 0.5f;
	int32_t i = *(int32_t*)&x;
	i = 0x5f3759df - (i >> 1);
	x = *(float*)&i;
	x = x * (1.5f - xhalf * x * x);
	return x;
}

extern uint32_t perf_start_count, perf_interval_count;

#define memset16(dest, val, count) \
	do { \
		uint32_t dummy1, dummy2; \
		asm volatile ( \
			"cld\n\t" \
			"test $1, %%ecx\n\t" \
			"jz 0f\n\t" \
			"rep stosw\n\t" \
			"jmp 1f\n\t" \
			"0:\n\t" \
			"shr $1, %%ecx\n\t" \
			"push %%ax\n\t" \
			"shl $16, %%eax\n\t" \
			"pop %%ax\n\t" \
			"rep stosl\n\t" \
			"1:\n\t"\
			: "=D"(dummy1), "=c"(dummy2) \
			: "0"(dest), "a"((uint16_t)(val)), "1"(count) \
			: "flags", "memory"); \
	} while(0)

/*
#define memset32(dest, val, count) \
	asm volatile ( \
		"cld\n\t" \
		"rep stosl\n\t" \
		:: "a"(val), "c"(count), "D"(dest) \
		: "flags", "memory")
*/
#define memset32(dest, val, count) \
	do { \
		int i; \
		uint32_t *ptr = (uint32_t*)dest; \
		for(i=0; i<count; i++) { \
			ptr[i] = val; \
		} \
	} while(0)


#ifdef USE_MMX
#define memcpy64(dest, src, count) asm volatile ( \
	"0:\n\t" \
	"movq (%1), %%mm0\n\t" \
	"movq %%mm0, (%0)\n\t" \
	"add $8, %1\n\t" \
	"add $8, %0\n\t" \
	"dec %2\n\t" \
	"jnz 0b\n\t" \
	"emms\n\t" \
	:: "r"(dest), "r"(src), "r"(count) \
	: "%mm0")
#else
#define memcpy64(dest, src, count)	memcpy(dest, src, (count) << 3)
#endif

#ifndef NO_PENTIUM
#define perf_start()  asm volatile ( \
	"xor %%eax, %%eax\n" \
	"cpuid\n" \
	"rdtsc\n" \
	"mov %%eax, %0\n" \
	: "=m"(perf_start_count) \
	:: "%eax", "%ebx", "%ecx", "%edx")

#define perf_end() asm volatile ( \
	"xor %%eax, %%eax\n" \
	"cpuid\n" \
	"rdtsc\n" \
	"sub %1, %%eax\n" \
	"mov %%eax, %0\n" \
	: "=m"(perf_interval_count) \
	: "m"(perf_start_count) \
	: "%eax", "%ebx", "%ecx", "%edx")
#endif	/* !def NO_PENTIUM */

#define debug_break() \
	asm volatile("int $3")

#define halt() \
	asm volatile("hlt")

unsigned int get_cs(void);
#define get_cpl()	((int)(get_cs() & 3))

void get_msr(uint32_t msr, uint32_t *low, uint32_t *high);
void set_msr(uint32_t msr, uint32_t low, uint32_t high);


/* Non-failing versions of malloc/calloc/realloc. They never return 0, they call
 * demo_abort on failure. Use the macros, don't call the *_impl functions.
 */
#define malloc_nf(sz)	malloc_nf_impl(sz, __FILE__, __LINE__)
void *malloc_nf_impl(size_t sz, const char *file, int line);
#define calloc_nf(n, sz)	calloc_nf_impl(n, sz, __FILE__, __LINE__)
void *calloc_nf_impl(size_t num, size_t sz, const char *file, int line);
#define realloc_nf(p, sz)	realloc_nf_impl(p, sz, __FILE__, __LINE__)
void *realloc_nf_impl(void *p, size_t sz, const char *file, int line);
#define strdup_nf(s)	strdup_nf_impl(s, __FILE__, __LINE__)
char *strdup_nf_impl(const char *s, const char *file, int line);

#endif	/* UTIL_H_ */
