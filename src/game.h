#ifndef GAME_H_
#define GAME_H_

#define FB_WIDTH	320
#define FB_HEIGHT	200

extern unsigned char *framebuf, *vmem;

int game_init(void);
void game_shutdown(void);

void game_draw(void);

void game_reshape(int x, int y);
void game_keyboard(int key, int press);
void game_mouse(int bn, int press, int x, int y);
void game_motion(int x, int y);

void game_quit(void);
void game_swap_buffers(void);

#endif	/* GAME_H_ */
