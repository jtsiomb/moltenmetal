#include "colormgr.h"
#include "vga.h"

/* TODO bring in a proper color manager with shade LUTs later */

void init_colormgr(void)
{
	unsigned int i, r, g, b;

	for(i=0; i<256; i++) {
		r = i & 0xe0;
		g = (i << 3) & 0xe0;
		b = (i << 5) & 0xc0;

		r |= r >> 3;
		g |= g >> 3;
		b |= (b >> 2) | (b >> 4);

		vga_setpalent(i, r, g, b);
	}
}

int find_color(int r, int g, int b)
{
	return (r & 0xe0) | ((g >> 3) & 0x1c) | ((b >> 6) & 3);
}

int shade_color(int col, int shade)
{
	return col;	/* TODO */
}
