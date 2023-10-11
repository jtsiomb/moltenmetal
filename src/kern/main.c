#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

	for(;;) {
		if((key = kb_getkey()) >= 0) {
			game_keyboard(key, 1);
		}

		mbn = mouse_state(&mx, &my);

		if((mbn_diff = mbn ^ prev_mbn) != 0) {
			if(mbn_diff & 1) {
				game_mouse(0, mbn & 1, mx, my);
			}
			if(mbn_diff & 2) {
				game_mouse(2, mbn & 2, mx, my);
			}
			if(mbn_diff & 4) {
				game_mouse(1, mbn & 4, mx, my);
			}
		}
		if(mx != prev_mx || my != prev_my) {
			game_motion(mx, my);
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
	draw_cursor(mx, my);

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
