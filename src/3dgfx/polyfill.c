#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "polyfill.h"
#include "gfxutil.h"
#include "util.h"
#include "colormgr.h"

/*#define DEBUG_OVERDRAW	G3D_PACK_RGB(10, 10, 10)*/

/* mode bits:
 *     bit 0: gouraud
 *     bit 1: texture
 *     bit 2: zbuffering
 */
void (*fillfunc[])(struct pvertex*) = {
	polyfill_flat,
	polyfill_gouraud,
	polyfill_tex_flat,
	polyfill_tex_gouraud,
	polyfill_flat_zbuf,
	polyfill_gouraud_zbuf,
	polyfill_tex_flat_zbuf,
	polyfill_tex_gouraud_zbuf
};

struct pimage pfill_fb, pfill_tex;
uint32_t *pfill_zbuf;
struct pgradient pgrad;

#define EDGEPAD	8
static struct pvertex *edgebuf, *left, *right;
static int edgebuf_size;
static int fbheight;

/*
#define CHECKEDGE(x) \
	do { \
		assert(x >= 0); \
		assert(x < fbheight); \
	} while(0)
*/
#define CHECKEDGE(x)


void polyfill_fbheight(int height)
{
	int newsz = (height * 2 + EDGEPAD * 3) * sizeof *edgebuf;

	if(newsz > edgebuf_size) {
		free(edgebuf);
		if(!(edgebuf = malloc(newsz))) {
			fprintf(stderr, "failed to allocate edge table buffer (%d bytes)\n", newsz);
			abort();
		}
		edgebuf_size = newsz;

		left = edgebuf + EDGEPAD;
		right = edgebuf + height + EDGEPAD * 2;

#ifndef NDEBUG
		memset(edgebuf, 0xaa, EDGEPAD * sizeof *edgebuf);
		memset(edgebuf + height + EDGEPAD, 0xaa, EDGEPAD * sizeof *edgebuf);
		memset(edgebuf + height * 2 + EDGEPAD * 2, 0xaa, EDGEPAD * sizeof *edgebuf);
#endif
	}

	fbheight = height;
}

void polyfill(int mode, struct pvertex *verts)
{
#ifndef NDEBUG
	if(!fillfunc[mode]) {
		fprintf(stderr, "polyfill mode %d not implemented\n", mode);
		abort();
	}
#endif

	fillfunc[mode](verts);
}

#define VNEXT(p)	(((p) == varr + 2) ? varr : (p) + 1)
#define VPREV(p)	((p) == varr ? varr + 2 : (p) - 1)
#define VSUCC(p, side)	((side) == 0 ? VNEXT(p) : VPREV(p))

/* extra bits of precision to use when interpolating colors.
 * try tweaking this if you notice strange quantization artifacts.
 */
#define COLOR_SHIFT	12


#define POLYFILL polyfill_flat
#undef GOURAUD
#undef TEXMAP
#undef ZBUF
#include "polytmpl.h"
#undef POLYFILL

#define POLYFILL polyfill_gouraud
#define GOURAUD
#undef TEXMAP
#undef ZBUF
#include "polytmpl.h"
#undef POLYFILL

#define POLYFILL polyfill_tex_flat
#undef GOURAUD
#define TEXMAP
#undef ZBUF
#include "polytmpl.h"
#undef POLYFILL

#define POLYFILL polyfill_tex_gouraud
#define GOURAUD
#define TEXMAP
#undef ZBUF
#include "polytmpl.h"
#undef POLYFILL

/* ---- zbuffer variants ----- */

#define POLYFILL polyfill_flat_zbuf
#undef GOURAUD
#undef TEXMAP
#define ZBUF
#include "polytmpl.h"
#undef POLYFILL

#define POLYFILL polyfill_gouraud_zbuf
#define GOURAUD
#undef TEXMAP
#define ZBUF
#include "polytmpl.h"
#undef POLYFILL

#define POLYFILL polyfill_tex_flat_zbuf
#undef GOURAUD
#define TEXMAP
#define ZBUF
#include "polytmpl.h"
#undef POLYFILL

#define POLYFILL polyfill_tex_gouraud_zbuf
#define GOURAUD
#define TEXMAP
#define ZBUF
#include "polytmpl.h"
#undef POLYFILL
