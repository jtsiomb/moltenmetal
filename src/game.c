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

#define BBOX_SIZE		10.0f
#define BBOX_HEIGHT		15.0f
#define BBOX_HSZ		(BBOX_SIZE / 2.0f)
#define BBOX_HH			(BBOX_HEIGHT / 2.0f)
#define VOX_RES			30
#define VOX_YRES		(VOX_RES * BBOX_HEIGHT / BBOX_SIZE)
#define VOX_STEP		(BBOX_SIZE / (float)VOX_RES)
#define VOX_YSTEP		(BBOX_HEIGHT / (float)VOX_YRES)

#define VBUF_MAX_TRIS	256
#define VBUF_SIZE		(VBUF_MAX_TRIS * 3)

struct mball {
	float energy;
	cgm_vec3 pos;
};

struct mcapsule {
	float energy;
	cgm_vec3 end[2];
	float len;
};

static struct g3d_mesh mesh;
static struct g3d_vertex *vbuf;
static struct metasurface *msurf;
static struct mball *balls;
static int num_balls;
static struct mcapsule *caps;
static int num_caps;

static void update(float tsec);
static void draw_metaballs(void);
static float capsule_distsq(struct mcapsule *c, cgm_vec3 *pos);

static cgm_vec3 sgiv[] = {
	{2.794170, 4.254175, 2.738066},
	{2.794170, 4.254174, -4.358471},
	{-2.173414, 4.254174, -4.358471},
	{-2.173414, -2.842363, -4.358470},
	{4.923134, -2.842363, -4.358471},
	{4.923134, 2.125212, -4.358471},
	{4.923134, 2.125212, 2.738066},
	{4.923134, -4.971326, 2.738067},
	{4.923134, -4.971326, -2.229511},
	{-2.173413, -4.971326, -2.229511},
	{-2.173413, -4.971325, 4.867042},
	{2.794170, -4.971325, 4.867042},
	{2.794170, 2.125213, 4.867042},
	{-4.302382, 2.125213, 4.867042},
	{-4.302383, -2.842362, 4.867042},
	{-4.302382, -2.842363, -2.229511},
	{-4.302382, 4.254175, -2.229512},
	{-4.302383, 4.254175, 2.738066}
};

int game_init(void)
{
	int i;
	float mat[16];

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

	gen_torus_mesh(&mesh, 2.0, 0.7, 24, 12);

	if(!(msurf = msurf_create())) {
		return -1;
	}
	msurf_set_threshold(msurf, 8);
	msurf_set_inside(msurf, MSURF_GREATER);
	msurf_set_bounds(msurf, -BBOX_HSZ, -BBOX_HH, -BBOX_HSZ, BBOX_HSZ, BBOX_HH, BBOX_HSZ);
	msurf_set_resolution(msurf, VOX_RES, VOX_YRES, VOX_RES);
	msurf_enable(msurf, MSURF_NORMALIZE);

	vbuf = malloc_nf(VBUF_SIZE * sizeof *vbuf);

	num_balls = 0;
	balls = calloc_nf(num_balls, sizeof *balls);
	num_caps = sizeof sgiv / sizeof *sgiv;
	caps = calloc_nf(num_caps, sizeof *caps);

	for(i=0; i<num_balls; i++) {
		balls[i].energy = 5;
	}

	cgm_midentity(mat);
	cgm_mtranslate(mat, 0, -BBOX_HH / 2, 0);
	cgm_mrotate_y(mat, -M_PI / 4.0f);
	cgm_mrotate_x(mat, M_PI / 4.0f);

	for(i=0; i<num_caps; i++) {
		caps[i].energy = 1;
		caps[i].end[0] = sgiv[i];
		cgm_vscale(caps[i].end, 0.6);
		caps[i].end[1] = sgiv[(i + 1) % num_caps];
		cgm_vscale(caps[i].end + 1, 0.6);

		cgm_vmul_m4v3(caps[i].end, mat);
		cgm_vmul_m4v3(caps[i].end + 1, mat);
	}
	return 0;
}

void game_shutdown(void)
{
}

static void update(float tsec)
{
	int i, j, k, n;
	float dsq, energy;
	cgm_vec3 pos;
	float *vox = msurf_voxels(msurf);

	for(i=0; i<num_balls; i++) {
		balls[i].pos.y = sin(tsec) * BBOX_HH;
	}

	float y = sin(tsec) * BBOX_HH / 80.0f;
	for(i=0; i<num_caps; i++) {
		caps[i].end[0].y += y;
		caps[i].end[1].y += y;
		caps[i].len = cgm_vdist(caps[i].end, caps[i].end + 1);
	}

	for(i=0; i<VOX_RES; i++) {
		pos.z = -BBOX_HSZ + i * VOX_STEP;
		for(j=0; j<VOX_YRES; j++) {
			pos.y = -BBOX_HH + j * VOX_YSTEP;
			for(k=0; k<VOX_RES; k++) {
				pos.x = -BBOX_HSZ + k * VOX_STEP;

				/* initialize with the vertical distance for the pool */
				energy = 5.0 / (pos.y + BBOX_HH * 0.98);

				/* add the contribution of the balls */
				for(n=0; n<num_balls; n++) {
					dsq = cgm_vdist_sq(&balls[n].pos, &pos);
					energy += balls[n].energy / dsq;
				}

				/* add the contribution of the capsules */
				for(n=0; n<num_caps; n++) {
					dsq = capsule_distsq(caps + n, &pos);
					energy += caps[n].energy / dsq;
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
	g3d_translate(0, 1, -15);
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

static float capsule_distsq(struct mcapsule *c, cgm_vec3 *pos)
{
	float t;
	cgm_vec3 pp, dir, pdir;

	dir = c->end[1]; cgm_vsub(&dir, c->end);
	if(c->len != 0.0f) {
		float s = 1.0f / c->len;
		dir.x *= s;
		dir.y *= s;
		dir.z *= s;
	}
	pdir = *pos; cgm_vsub(&pdir, c->end);
	t = cgm_vdot(&dir, &pdir);

	if(t < 0.0f) {
		return cgm_vdist_sq(c->end, pos);
	}
	if(t > c->len) {
		return cgm_vdist_sq(c->end + 1, pos);
	}

	pp = c->end[0];
	cgm_vadd_scaled(&pp, &dir, t);
	return cgm_vdist_sq(&pp, pos);
}
