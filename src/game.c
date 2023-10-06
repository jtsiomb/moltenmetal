#include <string.h>
#include "game.h"

int game_init(void)
{
	return 0;
}

void game_shutdown(void)
{
}

void game_draw(void)
{
	memset(framebuf, 2, 64000);

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
