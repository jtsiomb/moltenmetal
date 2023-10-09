#include <stdio.h>
#include <string.h>
#include <math.h>
#include "game.h"
#include "colormgr.h"
#include "3dgfx.h"
#include "mesh.h"
#include "metasurf.h"
#include "util.h"

#define BBOX_SIZE		10.0f
#define BBOX_HSZ		(BBOX_SIZE / 2.0f)
#define VOX_RES			30
#define VOX_STEP		(BBOX_SIZE / (float)VOX_RES)

#define VBUF_MAX_TRIS	256
#define VBUF_SIZE		(VBUF_MAX_TRIS * 3)

struct mball {
	float energy;
	float x, y, z;
};

static struct g3d_mesh mesh;
static struct g3d_vertex *vbuf;
static struct metasurface *msurf;
static struct mball *balls;
static int num_balls;

static void update(float tsec);
static void draw_metaballs(void);

int game_init(void)
{
	int i;

	init_colormgr();

	g3d_init();
	g3d_framebuffer(320, 200, framebuf);
	g3d_viewport(0, 0, 320, 200);

	g3d_clear_color(0, 0, 0);

	g3d_matrix_mode(G3D_PROJECTION);
	g3d_load_identity();
	g3d_perspective(60.0f, 1.33333, 0.5, 500.0);

	g3d_enable(G3D_CULL_FACE);
	g3d_enable(G3D_DEPTH_TEST);
	g3d_enable(G3D_LIGHTING);
	g3d_enable(G3D_LIGHT0);

	g3d_polygon_mode(G3D_GOURAUD);

	gen_torus_mesh(&mesh, 2.0, 0.7, 24, 12);

	if(!(msurf = msurf_create())) {
		return -1;
	}
	msurf_set_threshold(msurf, 8);
	msurf_set_inside(msurf, MSURF_GREATER);
	msurf_set_bounds(msurf, -BBOX_HSZ, -BBOX_HSZ, -BBOX_HSZ, BBOX_HSZ, BBOX_HSZ, BBOX_HSZ);
	msurf_set_resolution(msurf, VOX_RES, VOX_RES, VOX_RES);
	msurf_enable(msurf, MSURF_NORMALIZE);

	vbuf = malloc_nf(VBUF_SIZE * sizeof *vbuf);

	num_balls = 1;
	balls = calloc_nf(num_balls, sizeof *balls);

	for(i=0; i<num_balls; i++) {
		balls[i].energy = 20;
	}
	return 0;
}

void game_shutdown(void)
{
}

static void update(float tsec)
{
	int i, j, k, n;
	float x, y, z, dx, dy, dz, dsq, energy;
	float *vox = msurf_voxels(msurf);

	for(i=0; i<num_balls; i++) {
		balls[i].y = sin(tsec) * 5.0f;
	}

	for(i=0; i<VOX_RES; i++) {
		z = -BBOX_HSZ + i * VOX_STEP;
		for(j=0; j<VOX_RES; j++) {
			y = -BBOX_HSZ + j * VOX_STEP;
			for(k=0; k<VOX_RES; k++) {
				x = -BBOX_HSZ + k * VOX_STEP;

				/* initialize with the vertical distance for the pool */
				energy = 5.0 / (y + BBOX_HSZ * 0.98);

				/* add the contribution of the balls */
				for(n=0; n<num_balls; n++) {
					dx = x - balls[n].x;
					dy = y - balls[n].y;
					dz = z - balls[n].z;
					dsq = dx * dx + dy * dy + dz * dz;

					energy += balls[n].energy / dsq;
				}
				*vox++ = energy;
			}
		}
	}

	msurf_polygonize(msurf);
}

void game_draw(void)
{
	unsigned long msec = game_getmsec();
	float tsec = (float)msec / 1000.0f;

	update(tsec);

	g3d_clear(G3D_COLOR_BUFFER_BIT | G3D_DEPTH_BUFFER_BIT);

	g3d_matrix_mode(G3D_MODELVIEW);
	g3d_load_identity();
	g3d_translate(0, 0, -15);
	/*g3d_rotate(tsec * 50.0f, 1, 0, 0);
	g3d_rotate(tsec * 30.0f, 0, 0, 1);

	draw_mesh(&mesh);*/
	draw_metaballs();

	game_swap_buffers();
}

static void draw_metaballs(void)
{
	int i, nverts, vbuf_count;
	float *varr, *narr;
	struct g3d_vertex *vbptr;
	static int nfrm;

	nverts = msurf_vertex_count(msurf);
	varr = msurf_vertices(msurf);
	narr = msurf_normals(msurf);

	vbptr = vbuf;
	for(i=0; i<nverts; i++) {
		vbuf_count = vbptr - vbuf;
		if(vbuf_count >= VBUF_SIZE) {
			g3d_draw(G3D_TRIANGLES, vbuf, vbuf_count);
			vbptr = vbuf;
		}
		vbptr->x = varr[0];
		vbptr->y = varr[1];
		vbptr->z = varr[2];
		vbptr->w = 1.0f;
		vbptr->nx = narr[0];
		vbptr->ny = narr[1];
		vbptr->nz = narr[2];
		vbptr->w = 1.0f;
		vbptr->l = 255;
		vbptr++;
		varr += 3;
		narr += 3;
	}

	if(vbptr > vbuf) {
		g3d_draw(G3D_TRIANGLES, vbuf, vbptr - vbuf);
	}

	nfrm++;
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
