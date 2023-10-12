#include <stdio.h>
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

int find_color(int r, int g, int b)
{
	return 0;	/* TODO */
}

int shade_color(int col, int shade)
{
	return LOOKUP_SHADE(col, shade);
}
