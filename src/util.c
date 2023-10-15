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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include "util.h"
#include "panic.h"

uint32_t perf_start_count, perf_interval_count;

void *malloc_nf_impl(size_t sz, const char *file, int line)
{
	void *p;
	if(!(p = malloc(sz))) {
		panic("%s:%d failed to allocate %lu bytes\n", file, line, (unsigned long)sz);
	}
	return p;
}

void *calloc_nf_impl(size_t num, size_t sz, const char *file, int line)
{
	void *p;
	if(!(p = calloc(num, sz))) {
		panic("%s:%d failed to allocate %lu bytes\n", file, line, (unsigned long)(num * sz));
	}
	return p;
}

void *realloc_nf_impl(void *p, size_t sz, const char *file, int line)
{
	if(!(p = realloc(p, sz))) {
		panic("%s:%d failed to realloc %lu bytes\n", file, line, (unsigned long)sz);
	}
	return p;
}

char *strdup_nf_impl(const char *s, const char *file, int line)
{
	int len;
	char *res;

	len = strlen(s);
	if(!(res = malloc(len + 1))) {
		panic("%s:%d failed to duplicate string\n", file, line);
	}
	memcpy(res, s, len + 1);
	return res;
}
