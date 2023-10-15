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
#include <limits.h>
#include "segm.h"
#include "intr.h"
#include "mem.h"
#include "contty.h"
#include "keyb.h"
#include "psaux.h"
#include "timer.h"
#include "game.h"

unsigned char *framebuf, *vmem = (unsigned char*)0xa0000;

static int quit;
static int mx, my;
static long last_mouse_ev;

#define CURSOR_TIMEOUT	MSEC_TO_TICKS(3000)
static void draw_cursor(int mx, int my);


int main(void)
{
	int key;
	int prev_mx = 0, prev_my = 0;
	unsigned int mbn, prev_mbn = 0, mbn_diff;

	init_segm();
	init_intr();
	con_init();
	kb_init();
	init_psaux();
	init_mem();
	init_timer();
	enable_intr();

	if(!(framebuf = malloc(320 * 200))) {
		printf("failed to allocate framebuffer\n");
		goto end;
	}

	if(game_init() == -1) {
		goto end;
	}

	set_mouse_pos(160, 100);
	last_mouse_ev = LONG_MIN;

	for(;;) {
		if((key = kb_getkey()) >= 0) {
			game_keyboard(key, 1);
		}

		mbn = mouse_state(&mx, &my);

		if((mbn_diff = mbn ^ prev_mbn) != 0) {
			if(mbn_diff & 1) {
				game_mouse(0, mbn & 1, mx, my);
				last_mouse_ev = nticks;
			}
			if(mbn_diff & 2) {
				game_mouse(2, mbn & 2, mx, my);
				last_mouse_ev = nticks;
			}
			if(mbn_diff & 4) {
				game_mouse(1, mbn & 4, mx, my);
				last_mouse_ev = nticks;
			}
		}
		if(mx != prev_mx || my != prev_my) {
			game_motion(mx, my);
			last_mouse_ev = nticks;
		}
		prev_mbn = mbn;
		prev_mx = mx;
		prev_my = my;

		if(quit) break;
		game_draw();
	}
	game_shutdown();

end:
	cleanup_intr();	/* also disables interrupts */
	cleanup_timer();
	return 0;
}

unsigned long game_getmsec(void)
{
	return TICKS_TO_MSEC(nticks);
}

void game_quit(void)
{
	quit = 1;
}

void game_swap_buffers(void)
{
	if((long)nticks - last_mouse_ev < MSEC_TO_TICKS(3000)) {
		draw_cursor(mx, my);
	}

	memcpy(vmem, framebuf, 64000);
}

#define CUR_SZ	4

static void draw_cursor(int mx, int my)
{
	int i, sx, ex, sy, ey, xlen, ylen;
	unsigned char *ptr;

	sx = mx < CUR_SZ ? mx : mx - CUR_SZ;
	ex = mx >= 320 - CUR_SZ ? CUR_SZ - 1 : mx + CUR_SZ;
	xlen = ex - sx + 1;
	sy = my < CUR_SZ ? my : my - CUR_SZ;
	ey = my >= 200 - CUR_SZ ? CUR_SZ - 1 : my + CUR_SZ;
	ylen = ey - sy + 1;

	ptr = framebuf + (my << 8) + (my << 6) + sx;
	for(i=0; i<xlen; i++) {
		ptr[i] ^= 0xff;
	}

	ptr = framebuf + (sy << 8) + (sy << 6) + mx;
	for(i=0; i<ylen; i++) {
		*ptr ^= 0xff;
		ptr += 320;
	}
}
