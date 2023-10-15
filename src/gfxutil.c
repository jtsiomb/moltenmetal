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
#include <string.h>
#include <assert.h>
#include "game.h"
#include "gfxutil.h"
#include "3dgfx/3dgfx.h"

#if 0
enum {
	IN		= 0,
	LEFT	= 1,
	RIGHT	= 2,
	TOP		= 4,
	BOTTOM	= 8
};

static int outcode(int x, int y, int xmin, int ymin, int xmax, int ymax)
{
	int code = 0;

	if(x < xmin) {
		code |= LEFT;
	} else if(x > xmax) {
		code |= RIGHT;
	}
	if(y < ymin) {
		code |= TOP;
	} else if(y > ymax) {
		code |= BOTTOM;
	}
	return code;
}

#define FIXMUL(a, b)	(((a) * (b)) >> 8)
#define FIXDIV(a, b)	(((a) << 8) / (b))

#define LERP(a, b, t)	((a) + FIXMUL((b) - (a), (t)))

int clip_line(int *x0, int *y0, int *x1, int *y1, int xmin, int ymin, int xmax, int ymax)
{
	int oc_out;

	int oc0 = outcode(*x0, *y0, xmin, ymin, xmax, ymax);
	int oc1 = outcode(*x1, *y1, xmin, ymin, xmax, ymax);

	long fx0, fy0, fx1, fy1, fxmin, fymin, fxmax, fymax;

	if(!(oc0 | oc1)) return 1;	/* both points are inside */

	fx0 = *x0 << 8;
	fy0 = *y0 << 8;
	fx1 = *x1 << 8;
	fy1 = *y1 << 8;
	fxmin = xmin << 8;
	fymin = ymin << 8;
	fxmax = xmax << 8;
	fymax = ymax << 8;

	for(;;) {
		long x, y, t;

		if(oc0 & oc1) return 0;		/* both have points with the same outbit, not visible */
		if(!(oc0 | oc1)) break;		/* both points are inside */

		oc_out = oc0 ? oc0 : oc1;

		if(oc_out & TOP) {
			t = FIXDIV(fymin - fy0, fy1 - fy0);
			x = LERP(fx0, fx1, t);
			y = fymin;
		} else if(oc_out & BOTTOM) {
			t = FIXDIV(fymax - fy0, fy1 - fy0);
			x = LERP(fx0, fx1, t);
			y = fymax;
		} else if(oc_out & LEFT) {
			t = FIXDIV(fxmin - fx0, fx1 - fx0);
			x = fxmin;
			y = LERP(fy0, fy1, t);
		} else /*if(oc_out & RIGHT)*/ {
			t = FIXDIV(fxmax - fx0, fx1 - fx0);
			x = fxmax;
			y = LERP(fy0, fy1, t);
		}

		if(oc_out == oc0) {
			fx0 = x;
			fy0 = y;
			oc0 = outcode(fx0 >> 8, fy0 >> 8, xmin, ymin, xmax, ymax);
		} else {
			fx1 = x;
			fy1 = y;
			oc1 = outcode(fx1 >> 8, fy1 >> 8, xmin, ymin, xmax, ymax);
		}
	}

	*x0 = fx0 >> 8;
	*y0 = fy0 >> 8;
	*x1 = fx1 >> 8;
	*y1 = fy1 >> 8;
	return 1;
}

void draw_line(int x0, int y0, int x1, int y1, unsigned char color)
{
	int i, dx, dy, x_inc, y_inc, error;
	unsigned char *fb = framebuf;

	fb += y0 * FB_WIDTH + x0;

	dx = x1 - x0;
	dy = y1 - y0;

	if(dx >= 0) {
		x_inc = 1;
	} else {
		x_inc = -1;
		dx = -dx;
	}
	if(dy >= 0) {
		y_inc = FB_WIDTH;
	} else {
		y_inc = -FB_WIDTH;
		dy = -dy;
	}

	if(dx > dy) {
		error = dy * 2 - dx;
		for(i=0; i<=dx; i++) {
			*fb = color;
			if(error >= 0) {
				error -= dx * 2;
				fb += y_inc;
			}
			error += dy * 2;
			fb += x_inc;
		}
	} else {
		error = dx * 2 - dy;
		for(i=0; i<=dy; i++) {
			*fb = color;
			if(error >= 0) {
				error -= dy * 2;
				fb += x_inc;
			}
			error += dx * 2;
			fb += y_inc;
		}
	}
}
#endif

void draw_billboard(float x, float y, float z, float size, int lum, int a)
{
	float m[16];
	size *= 0.5f;

	g3d_matrix_mode(G3D_MODELVIEW);
	g3d_push_matrix();

	g3d_translate(x, y, z);

	g3d_get_matrix(G3D_MODELVIEW, m);
	/* make the upper 3x3 part of the matrix identity */
	m[0] = m[5] = m[10] = 1.0f;
	m[1] = m[2] = m[3] = m[4] = m[6] = m[7] = m[8] = m[9] = 0.0f;
	g3d_load_matrix(m);

	g3d_begin(G3D_QUADS);
	g3d_color2b(lum, a);
	g3d_texcoord(0, 0);
	g3d_vertex(-size, -size, 0);
	g3d_texcoord(1, 0);
	g3d_vertex(size, -size, 0);
	g3d_texcoord(1, 1);
	g3d_vertex(size, size, 0);
	g3d_texcoord(0, 1);
	g3d_vertex(-size, size, 0);
	g3d_end();

	g3d_pop_matrix();
}
