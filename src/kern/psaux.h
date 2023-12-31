/*
pcboot - bootable PC demo/game kernel
Copyright (C) 2018  John Tsiombikas <nuclear@member.fsf.org>

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
#ifndef PSAUX_H_
#define PSAUX_H_

#define MOUSE_LBN_BIT	1
#define MOUSE_RBN_BIT	2
#define MOUSE_MBN_BIT	4

void init_psaux(void);

int have_mouse(void);

void set_mouse_pos(int x, int y);
void set_mouse_bounds(int x0, int y0, int x1, int y1);
unsigned int mouse_state(int *xp, int *yp);

void poll_mouse(void);

#endif	/* PSAUX_H_ */
