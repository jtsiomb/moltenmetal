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


#define BBOX_HXSZ		(BBOX_XSZ / 2.0f)
#define BBOX_HYSZ		(BBOX_YSZ / 2.0f)
#define BBOX_HZSZ		(BBOX_ZSZ / 2.0f)
#define VOX_XRES		(VOX_RES * BBOX_XSZ / BBOX_ZSZ)
#define VOX_YRES		(VOX_RES * BBOX_YSZ / BBOX_ZSZ)
#define VOX_ZRES		VOX_RES
#define VOX_XSTEP		(BBOX_XSZ / (float)VOX_XRES)
#define VOX_YSTEP		(BBOX_YSZ / (float)VOX_YRES)
#define VOX_ZSTEP		(BBOX_ZSZ / (float)VOX_ZRES)

#define VBUF_MAX_TRIS	256
#define VBUF_SIZE		(VBUF_MAX_TRIS * 3)

unsigned long time_msec;

static struct g3d_vertex *vbuf;
static struct metasurface *msurf;
static struct mobject **mobjects, *mobj;

#define NUM_OBJ		2
static int num_mobj, cur_obj;

static int mousebn[3];
static int mousex, mousey;
static float cam_theta, cam_phi;

extern unsigned char textures_img[];
extern unsigned char textures_cmap[];
extern unsigned char textures_slut[];


static void update(float tsec);
static void draw_metaballs(void);


int game_init(void)
{
	init_colormgr();
	load_colormap(0, 256, textures_cmap, textures_slut);

	g3d_init();
	g3d_framebuffer(FB_WIDTH, FB_HEIGHT, framebuf);
	g3d_viewport(0, 0, FB_WIDTH, FB_HEIGHT);

	g3d_clear_color(0);

	g3d_matrix_mode(G3D_PROJECTION);
	g3d_load_identity();
	g3d_perspective(60.0f, 1.33333, 0.5, 500.0);

	g3d_enable(G3D_CULL_FACE);
	g3d_enable(G3D_DEPTH_TEST);
	g3d_enable(G3D_LIGHTING);
	g3d_enable(G3D_LIGHT0);
	g3d_light_ambient(0.2);

	g3d_polygon_mode(G3D_GOURAUD);

	if(!(msurf = msurf_create())) {
		return -1;
	}
	msurf_set_threshold(msurf, 8);
	msurf_set_inside(msurf, MSURF_GREATER);
	msurf_set_bounds(msurf, -BBOX_HXSZ, -BBOX_HYSZ, -BBOX_HZSZ, BBOX_HXSZ, BBOX_HYSZ, BBOX_HZSZ);
	msurf_set_resolution(msurf, VOX_XRES, VOX_YRES, VOX_ZRES);
	msurf_enable(msurf, MSURF_NORMALIZE);

	vbuf = malloc_nf(VBUF_SIZE * sizeof *vbuf);

	num_mobj = NUM_OBJ;
	mobjects = malloc(num_mobj * sizeof *mobj);
	mobjects[0] = metaobj_sflake();
	mobjects[1] = metaobj_sgi();
	cur_obj = 0;
	mobj = mobjects[cur_obj];
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

	mobjects[cur_obj]->update(mobjects[cur_obj], tsec);

	for(i=0; i<VOX_ZRES; i++) {
		pos.z = -BBOX_HZSZ + i * VOX_ZSTEP;
		for(j=0; j<VOX_YRES; j++) {
			pos.y = -BBOX_HYSZ + j * VOX_YSTEP;
			for(k=0; k<VOX_XRES; k++) {
				pos.x = -BBOX_HXSZ + k * VOX_XSTEP;

				/* initialize with the vertical distance for the pool */
				energy = 5.0 / (pos.y + BBOX_HYSZ);
				/*energy += 5.0 / (pos.x + BBOX_HXSZ);
				energy += 5.0 / (BBOX_HXSZ - pos.x);*/

				energy += mobj->eval(mobj, &pos);

				*vox++ = energy;
			}
		}
	}

	msurf_polygonize(msurf);
}

void game_draw(void)
{
	float tsec;

	time_msec = game_getmsec();
	tsec = (float)time_msec / 1000.0f;

	update(tsec);

	g3d_clear(G3D_COLOR_BUFFER_BIT | G3D_DEPTH_BUFFER_BIT);

	g3d_matrix_mode(G3D_MODELVIEW);
	g3d_load_identity();
	g3d_translate(0, 1, -14);
	g3d_rotate(cam_phi, 1, 0, 0);
	g3d_rotate(cam_theta, 0, 1, 0);

	g3d_disable(G3D_LIGHTING);
	g3d_enable(G3D_TEXTURE_2D);
	g3d_enable(G3D_TEXTURE_GEN);
	g3d_set_texture(32, 32, textures_img);
	draw_metaballs();
	g3d_disable(G3D_TEXTURE_GEN);
	g3d_enable(G3D_LIGHTING);

	game_swap_buffers();
}

static void draw_metaballs(void)
{
	int i, nverts, vbuf_count;
	float *varr, *narr;
	struct g3d_vertex *vbptr;

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
		if(press) {
			if(y > 3 * FB_HEIGHT / 4) {
				mobj->swstate(mobj, MOBJ_GRABING);
			}
		} else {
			mobj->swstate(mobj, MOBJ_DROPPING);
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
		mobj->pos.x += dx * 0.1;
		mobj->pos.y -= dy * 0.1;
	}
	if(mousebn[2]) {
		cam_theta += (float)dx * (0.6f * 1.333333333f);
		cam_phi += (float)dy * 0.6f;
		if(cam_phi < -90) cam_phi = -90;
		if(cam_phi > 90) cam_phi = 90;
	}
}
