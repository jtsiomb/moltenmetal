#include <string.h>
#include "game.h"
#include "colormgr.h"
#include "3dgfx.h"
#include "mesh.h"

static struct g3d_mesh mesh;

int game_init(void)
{
	init_colormgr();

	g3d_init();
	g3d_framebuffer(320, 200, framebuf);
	g3d_viewport(0, 0, 320, 200);

	g3d_clear_color(0, 0, 0);

	g3d_matrix_mode(G3D_PROJECTION);
	g3d_load_identity();
	g3d_perspective(50.0f, 1.33333, 0.5, 500.0);

	g3d_enable(G3D_CULL_FACE);
	g3d_enable(G3D_DEPTH_TEST);
	g3d_enable(G3D_LIGHTING);
	g3d_enable(G3D_LIGHT0);

	g3d_polygon_mode(G3D_GOURAUD);

	gen_torus_mesh(&mesh, 2.0, 0.7, 24, 12);
	return 0;
}

void game_shutdown(void)
{
}

void game_draw(void)
{
	unsigned long msec = game_getmsec();
	float tsec = (float)msec / 1000.0f;

	g3d_clear(G3D_COLOR_BUFFER_BIT | G3D_DEPTH_BUFFER_BIT);

	g3d_matrix_mode(G3D_MODELVIEW);
	g3d_load_identity();
	g3d_translate(0, 0, -8);
	g3d_rotate(tsec * 50.0f, 1, 0, 0);
	g3d_rotate(tsec * 30.0f, 0, 0, 1);

	draw_mesh(&mesh);

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
