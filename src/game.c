#include <string.h>
#include "game.h"
#include "colormgr.h"

int game_init(void)
{
	init_colormgr();
	return 0;
}

void game_shutdown(void)
{
}

void game_draw(void)
{
	int i, j;
	unsigned char *fbptr = framebuf;

	for(i=0; i<200; i++) {
		for(j=0; j<320; j++) {
			int r, b;
			int idx = i + (rand() & 0x1f) - 16;
			if(idx < 0) idx = 0;
			if(idx > 199) idx = 199;

			r = 255 * idx / 199;
			b = 255 - r;
			*fbptr++ = find_color(r, 0, b);
		}
	}

	game_swap_buffers();
}

void game_keyboard(int key, int press)
{
	if(key == 27) game_quit();
}

void game_mouse(int bn, int press, int x, int y)
{
}

void game_motion(int x, int y)
{
}
