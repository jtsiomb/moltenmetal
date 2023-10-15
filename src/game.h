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
#ifndef GAME_H_
#define GAME_H_

#define FB_WIDTH	320
#define FB_HEIGHT	200

#define BBOX_XSZ		16
#define BBOX_YSZ		15
#define BBOX_ZSZ		10
#define DEF_VOX_RES		24

#define TRANSDUR		1.0f

extern unsigned char *framebuf, *vmem;
extern unsigned long time_msec;

int game_init(void);
void game_shutdown(void);

void game_draw(void);

void game_reshape(int x, int y);
void game_keyboard(int key, int press);
void game_mouse(int bn, int press, int x, int y);
void game_motion(int x, int y);

unsigned long game_getmsec(void);
void game_quit(void);
void game_swap_buffers(void);

#endif	/* GAME_H_ */
