#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "game.h"
#include "colormgr.h"
#include "3dgfx.h"
#include "mesh.h"
#include "metasurf.h"
#include "util.h"
#include "cgmath/cgmath.h"
#include "metaobj.h"
#ifdef COM32
#include "vga.h"
#endif


#define TEXSZ	256

#define BBOX_HXSZ		(BBOX_XSZ / 2.0f)
#define BBOX_HYSZ		(BBOX_YSZ / 2.0f)
#define BBOX_HZSZ		(BBOX_ZSZ / 2.0f)
#define VOX_XRES		(VOX_RES * BBOX_XSZ / BBOX_ZSZ)
#define VOX_YRES		(VOX_RES * BBOX_YSZ / BBOX_ZSZ)
#define VOX_ZRES		VOX_RES
#define VOX_XSTEP		(BBOX_XSZ / (float)VOX_XRES)
#define VOX_YSTEP		(BBOX_YSZ / (float)VOX_YRES)
#define VOX_ZSTEP		(BBOX_ZSZ / (float)VOX_ZRES)

#define AUTO_HEIGHT		(BBOX_YSZ / 6.0f)

#define VBUF_MAX_TRIS	256
#define VBUF_SIZE		(VBUF_MAX_TRIS * 3)

unsigned long time_msec;

static struct g3d_mesh *mesh;
static struct g3d_vertex *vbuf;
static struct metasurface *msurf;
static struct mobject **mobjects, *mobj;

static int num_mobj, cur_obj, chobj = -1;

static int mousebn[3];
static int mousex, mousey;
static float cam_theta, cam_phi;

extern unsigned char textures_img[];
extern unsigned char textures_cmap[];
extern unsigned char textures_slut[];
extern unsigned char room_mesh[];

static unsigned char *envmap;
static unsigned char *texmap;


static void update(float tsec);
static void draw_metaballs(void);
static void conv_mesh(struct g3d_mesh *m, void *mdata);
static void gen_textures(void);


void intr_entry_fast_timer(void);


int game_init(void)
{
	init_colormgr();
	load_colormap(0, 256, textures_cmap, textures_slut);
#ifdef COM32
	unsigned char savcol[24];
	memcpy(savcol, colormap, sizeof savcol);
	vga_setpalent(7, 128, 128, 128);
#endif

	gen_textures();

	mesh = malloc_nf(sizeof *mesh);
	conv_mesh(mesh, room_mesh);

	//texmap = textures_img;
	envmap = textures_img + 32 * 32;

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

	num_mobj = 2;
	mobjects = malloc(num_mobj * sizeof *mobj);
	mobjects[0] = metaobj_sflake();
	mobjects[1] = metaobj_sgi();
	cur_obj = 0;
	mobj = mobjects[cur_obj];

#ifdef COM32
	vga_setpalent(0, savcol[0], savcol[1], savcol[2]);
	vga_setpalent(7, savcol[21], savcol[22], savcol[23]);
#endif
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

	if(chobj >= 0) {
		if(mobj->state == MOBJ_IDLE) {
			cur_obj = chobj;
			mobj = mobjects[cur_obj];
			chobj = -1;
			mobj->swstate(mobj, MOBJ_GRABING);
			mobj->pos.y = AUTO_HEIGHT;
		}
	}

	mobj->update(mobj, tsec);

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

	g3d_push_matrix();
	g3d_translate(0, -BBOX_HYSZ * 0.89, 0);
	g3d_disable(G3D_LIGHTING);
	g3d_enable(G3D_TEXTURE_2D);
	g3d_set_texture(TEXSZ, TEXSZ, texmap);
	g3d_polygon_mode(G3D_FLAT);
	draw_mesh(mesh);
	g3d_polygon_mode(G3D_GOURAUD);
	g3d_pop_matrix();

	g3d_push_matrix();
	g3d_translate(0.5f * BBOX_XSZ / VOX_XRES, 0, 0.5f * BBOX_ZSZ / VOX_ZRES);
	g3d_disable(G3D_LIGHTING);
	g3d_enable(G3D_TEXTURE_GEN);
	g3d_set_texture(32, 32, envmap);
	draw_metaballs();
	g3d_disable(G3D_TEXTURE_GEN);
	g3d_enable(G3D_LIGHTING);
	g3d_pop_matrix();

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
	if(!press) return;

	switch(key) {
	case 27:
		game_quit();
		break;

	case '\t':
		chobj = (cur_obj + 1) % num_mobj;
		mobj->swstate(mobj, MOBJ_DROPPING);
		break;

	case ' ':
		if(mobj->state == MOBJ_IDLE) {
			mobj->swstate(mobj, MOBJ_GRABING);
			mobj->pos.y = AUTO_HEIGHT;
		} else {
			mobj->swstate(mobj, MOBJ_DROPPING);
		}
		break;
	}
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
	float u, v;
	int dx = x - mousex;
	int dy = y - mousey;
	mousex = x;
	mousey = y;

	if((dx | dy) == 0) return;

	if(mousebn[0]) {
		mobj->pos.x += dx * 0.1;
		mobj->pos.y -= dy * 0.1;
	}

	u = (float)x / (float)FB_WIDTH;
	v = (float)y / (float)FB_HEIGHT;
	cam_theta = cgm_lerp(-15, 15, u);
	cam_phi = cgm_lerp(-15, 25, v);
}

struct mesh_header {
	float scale;
	uint16_t vcount, tricount;
	int16_t vdata[1];
} __attribute__((packed));

static void conv_mesh(struct g3d_mesh *m, void *mdata)
{
	int i;
	struct mesh_header *hdr;
	int16_t *vptr;
	uint8_t *uvptr;
	uint16_t *idxptr;
	struct g3d_vertex *vert;/*, *va, *vb, *vc;
	cgm_vec3 vab, vac, norm;*/

	m->prim = G3D_TRIANGLES;

	hdr = (struct mesh_header*)mdata;

	m->vcount = hdr->vcount;
	m->icount = hdr->tricount * 3;

	vptr = hdr->vdata;
	uvptr = (uint8_t*)(hdr->vdata + m->vcount * 3);
	idxptr = (uint16_t*)(uvptr + m->vcount * 2);

	m->varr = malloc(m->vcount * sizeof *m->varr);
	vert = m->varr;
	for(i=0; i<(int)m->vcount; i++) {
		vert[i].x = (float)vptr[0] / 256.0f * hdr->scale;
		vert[i].y = (float)vptr[1] / 256.0f * hdr->scale;
		vert[i].z = (float)vptr[2] / 256.0f * hdr->scale;
		vert[i].u = (float)uvptr[0] / 255.0f;
		vert[i].v = (float)uvptr[1] / 255.0f;
		vert[i].l = vert->a = 255;
		vptr += 3;
		uvptr += 2;
	}

	m->iarr = idxptr;

	/*
	for(i=0; i<hdr->tricount; i++) {
		assert(idxptr[0] != idxptr[1] && idxptr[1] != idxptr[2]);
		va = m->varr + idxptr[0];
		vb = m->varr + idxptr[1];
		vc = m->varr + idxptr[2];
		idxptr += 3;
		vab = *(cgm_vec3*)vb; cgm_vsub(&vab, (cgm_vec3*)va);
		vac = *(cgm_vec3*)vc; cgm_vsub(&vac, (cgm_vec3*)va);
		cgm_vcross(&norm, &vab, &vac);
		cgm_vnormalize(&norm);

		va->nx = vb->nx = vc->nx = norm.x;
		va->ny = vb->ny = vc->ny = norm.y;
		va->nz = vb->nz = vc->nz = norm.z;
	}
	*/

	m->mtl = malloc_nf(sizeof *m->mtl);
	init_g3dmtl(m->mtl);
}

static void vgrad(unsigned char *rgbimg, int x, int y, int xsz, int ysz,
		int r0, int g0, int b0, int r1, int g1, int b1)
{
	int i, j;

	rgbimg += (y * TEXSZ + x) * 3;

	for(i=0; i<ysz; i++) {
		int32_t t = (i << 8) / (ysz - 1);
		int r = ((r0 << 8) + (r1 - r0) * t) >> 8;
		int g = ((g0 << 8) + (g1 - g0) * t) >> 8;
		int b = ((b0 << 8) + (b1 - b0) * t) >> 8;
		for(j=0; j<xsz; j++) {
			rgbimg[0] = r;
			rgbimg[1] = g;
			rgbimg[2] = b;
			rgbimg += 3;
		}
		rgbimg += (TEXSZ - xsz) * 3;
	}
}

static void blob_hline(unsigned char *rgbimg, int x, int y, int w, int br, int bg, int bb, float energy)
{
	int i, j, r, g, b;
	float dx, dy, dsq, val;

	for(i=0; i<TEXSZ; i++) {
		dy = i - y;
		for(j=0; j<TEXSZ; j++) {
			if(j < x) {
				dx = x - j;
			} else if(j >= x + w) {
				dx = j - x - w;
			} else {
				dx = 0;
			}
			dsq = dx * dx + dy * dy;
			val = dsq == 0.0f ? 1.0f : energy / dsq;
			r = (int)rgbimg[0] + val * br;
			g = (int)rgbimg[1] + val * bg;
			b = (int)rgbimg[2] + val * bb;
			rgbimg[0] = r > 255 ? 255 : r;
			rgbimg[1] = g > 255 ? 255 : g;
			rgbimg[2] = b > 255 ? 255 : b;
			rgbimg += 3;
		}
	}
}

void gen_textures(void)
{
	int i, j, col;
	unsigned char *rgb, *rgbptr;
	unsigned char *pptr;

	/*FILE *fp = fopen("foo.ppm", "wb");
	fprintf(fp, "P6\n%d %d\n255\n", TEXSZ, TEXSZ);*/

	rgb = calloc_nf(3, TEXSZ * TEXSZ);
	texmap = malloc_nf(TEXSZ * TEXSZ);

	rgbptr = rgb;
	for(i=0; i<100; i++) {
		pptr = textures_img + ((i >> 3) << 5);
		for(j=0; j<TEXSZ; j++) {
			col = pptr[j >> 3];
			rgbptr[0] = colormap[col * 3];
			rgbptr[1] = colormap[col * 3 + 1];
			rgbptr[2] = colormap[col * 3 + 2];
			rgbptr += 3;
		}
	}

	vgrad(rgb, 0, 100, TEXSZ, 50, 21, 24, 37, 23, 26, 39);
	vgrad(rgb, 0, 150, TEXSZ, 5, 18, 21, 34, 36, 40, 59);
	vgrad(rgb, 0, 155, TEXSZ, 5, 36, 40, 59, 18, 21, 34);
	vgrad(rgb, 0, 160, TEXSZ, 12, 18, 21, 34, 36, 40, 59);
	vgrad(rgb, 0, 172, TEXSZ, 5, 18, 21, 34, 36, 40, 59);
	vgrad(rgb, 0, 177, TEXSZ, 5, 36, 40, 59, 18, 21, 34);
	vgrad(rgb, 0, 181, TEXSZ, 12, 36, 40, 59, 18, 21, 34);
	vgrad(rgb, 0, 193, TEXSZ, 5, 18, 21, 34, 36, 40, 59);
	vgrad(rgb, 0, 198, TEXSZ, 5, 36, 40, 59, 18, 21, 34);
	vgrad(rgb, 0, 203, TEXSZ, TEXSZ - 203, 33, 36, 54, 33, 36, 54);

	blob_hline(rgb, 38, 177, 58, 50, 200, 255, 10);
	blob_hline(rgb, 160, 177, 58, 50, 200, 255, 10);

	pptr = texmap;
	for(i=0; i<TEXSZ; i++) {
		for(j=0; j<TEXSZ; j++) {
			*pptr++ = find_color(rgb[0], rgb[1], rgb[2]);
			/*fputc(rgb[0], fp);
			fputc(rgb[1], fp);
			fputc(rgb[2], fp);*/
			rgb += 3;
		}
	}
	/*fclose(fp);*/
}
