#ifndef POLYFILL_H_
#define POLYFILL_H_

#include "inttypes.h"
#include "3dgfx.h"

#define POLYFILL_MODE_MASK	0x03
#define POLYFILL_TEX_BIT	0x04
#define POLYFILL_ZBUF_BIT	0x08

enum {
	POLYFILL_WIRE			= 0,
	POLYFILL_FLAT,
	POLYFILL_GOURAUD,

	POLYFILL_TEX_WIRE		= 4,
	POLYFILL_TEX_FLAT,
	POLYFILL_TEX_GOURAUD,

	POLYFILL_WIRE_ZBUF			= 16,
	POLYFILL_FLAT_ZBUF,
	POLYFILL_GOURAUD_ZBUF,

	POLYFILL_TEX_WIRE_ZBUF		= 20,
	POLYFILL_TEX_FLAT_ZBUF,
	POLYFILL_TEX_GOURAUD_ZBUF
};

/* projected vertices for the rasterizer */
struct pvertex {
	int32_t x, y; /* 24.8 fixed point */
	int32_t u, v; /* 16.16 fixed point */
	int32_t l, a;  /* int 0-255 */
	int32_t z;	/* 0-(2^24-1) */
};

struct pgradient {
	int32_t dudx, dudy, dvdx, dvdy;
	int32_t dldx, dldy, dadx, dady;
	int32_t dzdx, dzdy;
};

struct pimage {
	g3d_pixel *pixels;
	int width, height;

	int xshift, yshift;
	unsigned int xmask, ymask;
};

extern struct pimage pfill_fb;
extern struct pimage pfill_tex;
extern uint32_t *pfill_zbuf;
extern struct pgradient pgrad;

void polyfill_fbheight(int height);

void polyfill(int mode, struct pvertex *verts);

void polyfill_wire(struct pvertex *verts);
void polyfill_flat(struct pvertex *verts);
void polyfill_gouraud(struct pvertex *verts);
void polyfill_tex_wire(struct pvertex *verts);
void polyfill_tex_flat(struct pvertex *verts);
void polyfill_tex_gouraud(struct pvertex *verts);
void polyfill_flat_zbuf(struct pvertex *verts);
void polyfill_gouraud_zbuf(struct pvertex *verts);
void polyfill_tex_flat_zbuf(struct pvertex *verts);
void polyfill_tex_gouraud_zbuf(struct pvertex *verts);

#endif	/* POLYFILL_H_ */
