#ifndef THREEDGFX_H_
#define THREEDGFX_H_

#include <inttypes.h>

typedef unsigned char g3d_pixel;


struct g3d_vertex {
	float x, y, z, w;
	float nx, ny, nz;
	float u, v;
	unsigned char l, a;
};

enum {
	G3D_POINTS = 1,
	G3D_LINES = 2,
	G3D_TRIANGLES = 3,
	G3D_QUADS = 4
};

/* g3d_enable/g3d_disable bits */
enum {
	G3D_CULL_FACE	= 0x000001,
	G3D_DEPTH_TEST	= 0x000002,
	G3D_LIGHTING	= 0x000004,
	G3D_LIGHT0		= 0x000008,
	G3D_LIGHT1		= 0x000010,
	G3D_LIGHT2		= 0x000020,
	G3D_LIGHT3		= 0x000040,
	G3D_TEXTURE_2D	= 0x000080,
	G3D_ALPHA_BLEND	= 0x000100,
	G3D_TEXTURE_GEN	= 0x000200,
	G3D_CLIP_FRUSTUM = 0x000800,/* when disabled, don't clip against the frustum */
	G3D_CLIP_PLANE0 = 0x001000,	/* user-defined 3D clipping planes XXX not impl. */
	G3D_CLIP_PLANE1 = 0x002000,
	G3D_CLIP_PLANE2 = 0x004000,
	G3D_CLIP_PLANE3 = 0x008000,

	G3D_TEXTURE_MAT	= 0x010000,
	G3D_SPECULAR	= 0x020000,

	G3D_ADD_BLEND	= 0x040000,

	G3D_ALL = 0x7fffffff
};

/* arg to g3d_front_face */
enum { G3D_CCW, G3D_CW };

/* arg to g3d_polygon_mode */
enum {
	G3D_FLAT,
	G3D_GOURAUD
};

/* matrix stacks */
enum {
	G3D_MODELVIEW,
	G3D_PROJECTION,
	G3D_TEXTURE,

	G3D_NUM_MATRICES
};

/* clear bits */
enum {
	G3D_COLOR_BUFFER_BIT = 1,
	G3D_DEPTH_BUFFER_BIT = 2
};

int g3d_init(void);
void g3d_destroy(void);
void g3d_reset(void);

void g3d_framebuffer(int width, int height, void *pixels);
void g3d_framebuffer_addr(void *pixels);
void g3d_viewport(int x, int y, int w, int h);

void g3d_clear_color(int cidx);
void g3d_clear_depth(float z);
void g3d_clear(unsigned int mask);

void g3d_enable(unsigned int opt);
void g3d_disable(unsigned int opt);
void g3d_setopt(unsigned int opt, unsigned int mask);
unsigned int g3d_getopt(unsigned int mask);

void g3d_front_face(unsigned int order);
void g3d_polygon_mode(int pmode);
int g3d_get_polygon_mode(void);

void g3d_matrix_mode(int mmode);

void g3d_load_identity(void);
void g3d_load_matrix(const float *m);
void g3d_mult_matrix(const float *m);
void g3d_push_matrix(void);
void g3d_pop_matrix(void);

void g3d_translate(float x, float y, float z);
void g3d_rotate(float angle, float x, float y, float z);
void g3d_scale(float x, float y, float z);
void g3d_ortho(float left, float right, float bottom, float top, float znear, float zfar);
void g3d_frustum(float left, float right, float bottom, float top, float znear, float zfar);
void g3d_perspective(float vfov, float aspect, float znear, float zfar);

/* returns pointer to the *internal* matrix, and if argument m is not null,
 * also copies the internal matrix there. */
const float *g3d_get_matrix(int which, float *m);

void g3d_light_pos(int idx, float x, float y, float z);
void g3d_light_dir(int idx, float x, float y, float z);
void g3d_light_energy(int idx, float val);

void g3d_light_ambient(float val);

void g3d_mtl_diffuse(float diffuse);
void g3d_mtl_specular(float spec);
void g3d_mtl_shininess(float shin);

void g3d_set_texture(int xsz, int ysz, void *pixels);

void g3d_draw(int prim, const struct g3d_vertex *varr, int varr_size);
void g3d_draw_indexed(int prim, const struct g3d_vertex *varr, int varr_size,
		const uint16_t *iarr, int iarr_size);

void g3d_begin(int prim);
void g3d_end(void);
void g3d_vertex(float x, float y, float z);
void g3d_normal(float x, float y, float z);
void g3d_color1b(unsigned char lum);
void g3d_color2b(unsigned char lum, unsigned char a);
void g3d_color1f(float lum);
void g3d_color2f(float lum, float a);
void g3d_texcoord(float u, float v);

#endif	/* THREEDGFX_H_ */
