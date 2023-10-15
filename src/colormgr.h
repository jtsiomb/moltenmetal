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
#ifndef COLORMGR_H_
#define COLORMGR_H_

#define SHADE_LEVELS	8
#define SHADE_SHIFT		3

#define LOOKUP_SHADE(col, shade) \
	shade_lut[(col << SHADE_SHIFT) + ((255 - (shade)) >> (8 - SHADE_SHIFT))]

extern unsigned char *colormap;
extern int *shade_lut;

void init_colormgr(void);

void load_colormap(int offs, int sz, unsigned char *col, unsigned char *slut);

int find_color(int r, int g, int b);
int shade_color(int col, int shade);	/* both 0-255 */

#endif	/* COLORMGR_H_ */
