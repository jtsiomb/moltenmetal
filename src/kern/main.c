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


int main(void)
{
	int key;

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

	for(;;) {
		if((key = kb_getkey()) >= 0) {
			game_keyboard(key, 1);
		}
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
	memcpy(vmem, framebuf, 64000);
}
