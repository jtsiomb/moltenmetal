#include <stdio.h>
#include <string.h>
#include <math.h>
#include "game.h"
#include "colormgr.h"
#include "3dgfx.h"
#include "mesh.h"
#include "metasurf.h"
#include "util.h"
#include "cgmath/cgmath.h"
#include "metaobj.h"

#define BBOX_SIZE		10.0f
#define BBOX_HEIGHT		15.0f
#define BBOX_HSZ		(BBOX_SIZE / 2.0f)
#define BBOX_HH			(BBOX_HEIGHT / 2.0f)
#define VOX_RES			32
#define VOX_YRES		(VOX_RES * BBOX_HEIGHT / BBOX_SIZE)
#define VOX_STEP		(BBOX_SIZE / (float)VOX_RES)
#define VOX_YSTEP		(BBOX_HEIGHT / (float)VOX_YRES)

#define VBUF_MAX_TRIS	256
#define VBUF_SIZE		(VBUF_MAX_TRIS * 3)

static struct g3d_vertex *vbuf;
static struct metasurface *msurf;
static struct mobject **mobj;

#define NUM_OBJ		2
static int num_mobj, cur_obj;
static int grabbed;

static int mousebn[3];
static int mousex, mousey;
static float cam_theta, cam_phi;


static void update(float tsec);
static void draw_metaballs(void);


int game_init(void)
{
	init_colormgr();

	g3d_init();
	g3d_framebuffer(FB_WIDTH, FB_HEIGHT, framebuf);
	g3d_viewport(0, 0, FB_WIDTH, FB_HEIGHT);

	g3d_clear_color(0, 0, 0);

	g3d_matrix_mode(G3D_PROJECTION);
	g3d_load_identity();
	g3d_perspective(60.0f, 1.33333, 0.5, 500.0);

	g3d_enable(G3D_CULL_FACE);
	g3d_enable(G3D_DEPTH_TEST);
	g3d_enable(G3D_LIGHTING);
	g3d_enable(G3D_LIGHT0);

	g3d_polygon_mode(G3D_GOURAUD);

	if(!(msurf = msurf_create())) {
		return -1;
	}
	msurf_set_threshold(msurf, 8);
	msurf_set_inside(msurf, MSURF_GREATER);
	msurf_set_bounds(msurf, -BBOX_HSZ, -BBOX_HH, -BBOX_HSZ, BBOX_HSZ, BBOX_HH, BBOX_HSZ);
	msurf_set_resolution(msurf, VOX_RES, VOX_YRES, VOX_RES);
	msurf_enable(msurf, MSURF_NORMALIZE);

	vbuf = malloc_nf(VBUF_SIZE * sizeof *vbuf);

	num_mobj = NUM_OBJ;
	mobj = malloc(num_mobj * sizeof *mobj);
	mobj[0] = metaobj_sgi();
	mobj[1] = metaobj_sflake();
	cur_obj = 1;
	return 0;
}

void game_shutdown(void)
{
}

static void update(float tsec)
{
	int i, j, k;
	float energy;
	cgm_vec3 pos;
	float *vox = msurf_voxels(msurf);

	mobj[cur_obj]->update(mobj[cur_obj], tsec);

	for(i=0; i<VOX_RES; i++) {
		pos.z = -BBOX_HSZ + i * VOX_STEP;
		for(j=0; j<VOX_YRES; j++) {
			pos.y = -BBOX_HH + j * VOX_YSTEP;
			for(k=0; k<VOX_RES; k++) {
				pos.x = -BBOX_HSZ + k * VOX_STEP;

				/* initialize with the vertical distance for the pool */
				energy = 5.0 / (pos.y + BBOX_HH * 0.98);

				energy += mobj[cur_obj]->eval(mobj[cur_obj], &pos);

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
	g3d_translate(0, 1, -15);
	g3d_rotate(cam_phi, 1, 0, 0);
	g3d_rotate(cam_theta, 0, 1, 0);
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
	mousebn[bn] = press;
	mousex = x;
	mousey = y;

	if(bn == 0) {
		if(press && !grabbed) {
			grabbed = 1;
		} else if(!press && grabbed) {
			grabbed = 0;
		}
	}
}

void game_motion(int x, int y)
{
	int dx = x - mousex;
	int dy = y - mousey;
	mousex = x;
	mousey = y;

	if((dx | dy) == 0) return;

	if(mousebn[0]) {
		if(grabbed) {
			mobj[cur_obj]->pos.x += dx * 0.1;
			mobj[cur_obj]->pos.y -= dy * 0.1;
		}
	}
	if(mousebn[2]) {
		cam_theta += (float)dx * (0.6f * 1.333333333f);
		cam_phi += (float)dy * 0.6f;
		if(cam_phi < -90) cam_phi = -90;
		if(cam_phi > 90) cam_phi = 90;
	}
}
