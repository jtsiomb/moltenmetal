#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <assert.h>
#include "miniglut.h"
#include "game.h"
#include "kern/keyb.h"

static void swap_interval(int vsync);
static void idle(void);
static void reshape(int x, int y);
static void keydown(unsigned char key, int x, int y);
static void keyup(unsigned char key, int x, int y);
static void skeydown(int key, int x, int y);
static void skeyup(int key, int x, int y);
static void mouse(int bn, int st, int x, int y);
static void motion(int x, int y);
static int translate_skey(int key);
static unsigned int next_pow2(unsigned int x);

#if defined(__unix__) || defined(unix)
#include <GL/glx.h>
static Display *xdpy;
static Window xwin;

static void (*glx_swap_interval_ext)();
static void (*glx_swap_interval_sgi)();
#endif
#ifdef _WIN32
#include <windows.h>
static PROC wgl_swap_interval_ext;
#endif

#ifndef GL_BGRA
#define GL_BGRA	0x80e1
#endif
#ifndef GL_CLAMP_TO_EDGE
#define GL_CLAMP_TO_EDGE	0x812f
#endif

unsigned char *framebuf;
static unsigned int tex;
static int tex_xsz, tex_ysz;

static int win_width, win_height;
static float win_aspect;
static float scale = 3;
static unsigned long start_time;
static uint32_t cmap[256];

int main(int argc, char **argv)
{
	glutInit(&argc, argv);

	glutInitWindowSize(FB_WIDTH * scale, FB_HEIGHT * scale);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
	glutCreateWindow("Molten Metal");

	glutDisplayFunc(game_draw);
	glutIdleFunc(idle);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keydown);
	glutKeyboardUpFunc(keyup);
	glutSpecialFunc(skeydown);
	glutSpecialUpFunc(skeyup);
	glutMouseFunc(mouse);
	glutMotionFunc(motion);
	glutPassiveMotionFunc(motion);

#if defined(__unix__) || defined(unix)
	xdpy = glXGetCurrentDisplay();
	xwin = glXGetCurrentDrawable();

	if(!(glx_swap_interval_ext = glXGetProcAddress((unsigned char*)"glXSwapIntervalEXT"))) {
		glx_swap_interval_sgi = glXGetProcAddress((unsigned char*)"glXSwapIntervalSGI");
	}
#endif
#ifdef _WIN32
	wgl_swap_interval_ext = wglGetProcAddress("wglSwapIntervalEXT");
#endif
	swap_interval(1);

	if(!(framebuf = malloc(FB_WIDTH * FB_HEIGHT))) {
		fprintf(stderr, "failed to allocate framebuffer\n");
		return 1;
	}

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	tex_xsz = next_pow2(FB_WIDTH);
	tex_ysz = next_pow2(FB_HEIGHT);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex_xsz, tex_ysz, 0, GL_RGBA,
			GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();
	glScalef((float)FB_WIDTH / tex_xsz, (float)FB_HEIGHT / tex_ysz, 1);
	glEnable(GL_TEXTURE_2D);

	if(game_init() == -1) {
		return 1;
	}
	atexit(game_shutdown);

	start_time = glutGet(GLUT_ELAPSED_TIME);
	glutMainLoop();
	return 0;
}

unsigned long game_getmsec(void)
{
	return (unsigned long)glutGet(GLUT_ELAPSED_TIME) - start_time;
}

void game_quit(void)
{
	exit(0);
}

#define FB_ASPECT	((float)FB_WIDTH / (float)FB_HEIGHT)

void game_swap_buffers(void)
{
	int i;
	static uint32_t fbrgba[FB_WIDTH * FB_HEIGHT * 4];

	for(i=0; i<FB_WIDTH * FB_HEIGHT; i++) {
		int cidx = framebuf[i];
		fbrgba[i] = cmap[cidx];
	}

	glBindTexture(GL_TEXTURE_2D, tex);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, FB_WIDTH, FB_HEIGHT, GL_RGBA,
			GL_UNSIGNED_BYTE, fbrgba);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	if(win_aspect >= FB_ASPECT) {
		glScalef(FB_ASPECT / win_aspect, 1, 1);
	} else {
		glScalef(1, win_aspect / FB_ASPECT, 1);
	}

	glClear(GL_COLOR_BUFFER_BIT);

	glBegin(GL_QUADS);
	glTexCoord2f(0, 1);
	glVertex2f(-1, -1);
	glTexCoord2f(1, 1);
	glVertex2f(1, -1);
	glTexCoord2f(1, 0);
	glVertex2f(1, 1);
	glTexCoord2f(0, 0);
	glVertex2f(-1, 1);
	glEnd();

	glutSwapBuffers();
	assert(glGetError() == GL_NO_ERROR);
}

void vga_setpalent(int idx, int r, int g, int b)
{
	cmap[idx] = (b << 16) | (g << 8) | r;
}

void vga_setpal(int startidx, int count, unsigned char *col)
{
	int i, idx = startidx;
	for(i=0; i<count; i++) {
		vga_setpalent(idx++, col[0], col[1], col[2]);
		col += 3;
	}
}

void panic(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
}

#if defined(__unix__) || defined(unix)
static void swap_interval(int vsync)
{
	vsync = vsync ? 1 : 0;
	if(glx_swap_interval_ext) {
		glx_swap_interval_ext(xdpy, xwin, vsync);
	} else if(glx_swap_interval_sgi) {
		glx_swap_interval_sgi(vsync);
	}
}
#endif
#ifdef WIN32
static void swap_interval(int vsync)
{
	if(wgl_swap_interval_ext) {
		wgl_swap_interval_ext(vsync ? 1 : 0);
	}
}
#endif

static void idle(void)
{
	glutPostRedisplay();
}

static void reshape(int x, int y)
{
	win_width = x;
	win_height = y;
	win_aspect = (float)x / (float)y;
	glViewport(0, 0, x, y);
}

static void keydown(unsigned char key, int x, int y)
{
	/*modkeys = glutGetModifiers();*/
	game_keyboard(key, 1);
}

static void keyup(unsigned char key, int x, int y)
{
	game_keyboard(key, 0);
}

static void skeydown(int key, int x, int y)
{
	int k;
	/*modkeys = glutGetModifiers();*/
	if((k = translate_skey(key)) >= 0) {
		game_keyboard(k, 1);
	}
}

static void skeyup(int key, int x, int y)
{
	int k = translate_skey(key);
	if(k >= 0) {
		game_keyboard(k, 0);
	}
}

static void mouse(int bn, int st, int x, int y)
{
	/*modkeys = glutGetModifiers();*/
	game_mouse(bn - GLUT_LEFT_BUTTON, st == GLUT_DOWN, x / scale, y / scale);
}

static void motion(int x, int y)
{
	game_motion(x / scale, y / scale);
}

static int translate_skey(int key)
{
	switch(key) {
	case GLUT_KEY_LEFT:
		return KB_LEFT;
	case GLUT_KEY_UP:
		return KB_UP;
	case GLUT_KEY_RIGHT:
		return KB_RIGHT;
	case GLUT_KEY_DOWN:
		return KB_DOWN;
	case GLUT_KEY_PAGE_UP:
		return KB_PGUP;
	case GLUT_KEY_PAGE_DOWN:
		return KB_PGDN;
	case GLUT_KEY_HOME:
		return KB_HOME;
	case GLUT_KEY_END:
		return KB_END;
	case GLUT_KEY_INSERT:
		return KB_INSERT;
	default:
		if(key >= GLUT_KEY_F1 && key <= GLUT_KEY_F12) {
			return key - GLUT_KEY_F1 + KB_F1;
		}
	}

	return -1;
}

static unsigned int next_pow2(unsigned int x)
{
	x--;
	x |= x >> 1;
	x |= x >> 2;
	x |= x >> 4;
	x |= x >> 8;
	x |= x >> 16;
	return x + 1;
}

