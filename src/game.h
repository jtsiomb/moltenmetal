#ifndef GAME_H_
#define GAME_H_

#define FB_WIDTH	320
#define FB_HEIGHT	200

#define BBOX_XSZ		16
#define BBOX_YSZ		15
#define BBOX_ZSZ		10
#define VOX_RES			24

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
