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
#include <limits.h>
#include "colormgr.h"
#include "vga.h"
#include "util.h"

unsigned char *colormap;
int *shade_lut;

void init_colormgr(void)
{
	unsigned int i, r, g, b;
	unsigned char *cptr;

	cptr = colormap = malloc_nf(256 * 3);

	for(i=0; i<256; i++) {
		r = i & 0xe0;
		g = (i << 3) & 0xe0;
		b = (i << 5) & 0xc0;

		r |= r >> 3;
		g |= g >> 3;
		b |= (b >> 2) | (b >> 4);

		cptr[0] = r | (r >> 3);
		cptr[1] = g | (g >> 3);
		cptr[2] = b | (b >> 2) | (b >> 4);
		vga_setpalent(i, cptr[0], cptr[1], cptr[2]);
		cptr += 3;
	}

	shade_lut = calloc_nf(256 * SHADE_LEVELS, sizeof *shade_lut);
}

void load_colormap(int offs, int sz, unsigned char *col, unsigned char *slut)
{
	int i, j;
	unsigned char *cptr;
	int *sptr;

	if(sz + offs > 256) sz = 256 - offs;

	cptr = colormap + offs * 3;
	sptr = shade_lut + (offs << SHADE_SHIFT);
	for(i=0; i<sz; i++) {
		cptr[0] = col[0];
		cptr[1] = col[1];
		cptr[2] = col[2];
		vga_setpalent(offs + i, col[0], col[1], col[2]);
		cptr += 3;
		col += 3;

		for(j=0; j<SHADE_LEVELS; j++) {
			sptr[j] = (int)slut[j];
		}
		sptr += SHADE_LEVELS;
		slut += SHADE_LEVELS;
	}
}

/* TODO optimize */
int find_color(int r, int g, int b)
{
	int i, best_col, best_dist, dr, dg, db, dsq;
	unsigned char *cmap = colormap;

	best_dist = INT_MAX;
	for(i=0; i<256; i++) {
		dr = r - cmap[0];
		dg = g - cmap[1];
		db = b - cmap[2];
		dsq = dr * dr + dg * dg + db * db;
		if(dsq < best_dist) {
			best_dist = dsq;
			best_col = i;
		}
		cmap += 3;
	}

	return best_col;
}

int shade_color(int col, int shade)
{
	return LOOKUP_SHADE(col, shade);
}
