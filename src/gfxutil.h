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
#ifndef GFXUTIL_H_
#define GFXUTIL_H_

#define PACK_RGB32(r, g, b) \
	((((r) & 0xff) << 16) | (((g) & 0xff) << 8) | ((b) & 0xff) | 0xff000000)

#define UNPACK_R32(c)	(((c) >> 16) & 0xff)
#define UNPACK_G32(c)	(((c) >> 8) & 0xff)
#define UNPACK_B32(c)	((c) & 0xff)


int clip_line(int *x0, int *y0, int *x1, int *y1, int xmin, int ymin, int xmax, int ymax);
void draw_line(int x0, int y0, int x1, int y1, unsigned char color);

void draw_billboard(float x, float y, float z, float size, int lum, int a);

#endif	/* GFXUTIL_H_ */
