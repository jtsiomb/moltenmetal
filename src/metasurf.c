/* Molten Metal - Tech demo for the COM32 DOS protected mode system
 * Copyright (C) 2023  John Tsiombikas <nuclear@mutantstargoat.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "metasurf.h"
#include "mcubes.h"
#include "util.h"

typedef float vec3[3];

struct metasurface {
	vec3 min, max;
	int res[3], newres[3];
	float thres;

	float dx, dy, dz;
	unsigned int flags;

	float *voxels;

	int varr_size, varr_alloc_size;
	float *varr, *narr;
};

static int msurf_init(struct metasurface *ms);
static void process_cell(struct metasurface *ms, int xcell, int ycell, int zcell, vec3 pos, vec3 sz);

static int **mc_tri_table;

static int decompress_tables(void)
{
	int i, j, run, nib, *data;
	unsigned char *indata;

	if(!(mc_tri_table = malloc(256 * 16 * sizeof(int) + 256 * sizeof(void*)))) {
		fprintf(stderr, "decompress_tables: failed to allocate memory\n");
		return -1;
	}

	indata = tritab_data;
	nib = 0;
	data = (int*)(mc_tri_table + 256);
	for(i=0; i<256; i++) {
		mc_tri_table[i] = data + i * 16;

		run = ((i & 1) == 0) ? tritab_runlen[i/2] & 0xf : tritab_runlen[i/2] >> 4;
		for(j=0; j<16; j++) {
			if(j < run) {
				mc_tri_table[i][j] = ((nib++ & 1) == 0) ? *indata & 0xf : *indata++ >> 4;
			} else {
				mc_tri_table[i][j] = -1;
			}
		}
	}

	return 0;
}

struct metasurface *msurf_create(void)
{
	struct metasurface *ms;

	if(!mc_tri_table) {
		if(decompress_tables() == -1) {
			return 0;
		}
	}

	if(!(ms = malloc(sizeof *ms))) {
		return 0;
	}
	if(msurf_init(ms) == -1) {
		free(ms);
	}
	return ms;
}

void msurf_free(struct metasurface *ms)
{
	if(ms) {
		free(ms->voxels);
		free(ms->varr);
		free(ms->narr);
		free(ms);
	}
}

static int msurf_init(struct metasurface *ms)
{
	ms->voxels = 0;
	ms->thres = 0.0;
	ms->min[0] = ms->min[1] = ms->min[2] = -1.0;
	ms->max[0] = ms->max[1] = ms->max[2] = 1.0;
	ms->res[0] = ms->res[1] = ms->res[2] = 0;
	ms->newres[0] = ms->newres[1] = ms->newres[2] = 40;

	ms->varr_alloc_size = ms->varr_size = 0;
	ms->varr = ms->narr = 0;

	ms->dx = ms->dy = ms->dz = 0.001;
	ms->flags = 0;

	return 0;
}

void msurf_enable(struct metasurface *ms, unsigned int opt)
{
	ms->flags |= opt;
}

void msurf_disable(struct metasurface *ms, unsigned int opt)
{
	ms->flags &= ~opt;
}

int msurf_is_enabled(struct metasurface *ms, unsigned int opt)
{
	return ms->flags & opt;
}

void msurf_set_inside(struct metasurface *ms, int inside)
{
	switch(inside) {
	case MSURF_GREATER:
		msurf_enable(ms, MSURF_FLIP);
		break;

	case MSURF_LESS:
		msurf_disable(ms, MSURF_FLIP);
		break;

	default:
		fprintf(stderr, "msurf_inside expects MSURF_GREATER or MSURF_LESS\n");
	}
}

int msurf_get_inside(struct metasurface *ms)
{
	return msurf_is_enabled(ms, MSURF_FLIP) ? MSURF_LESS : MSURF_GREATER;
}

void msurf_set_bounds(struct metasurface *ms, float xmin, float ymin, float zmin, float xmax, float ymax, float zmax)
{
	ms->min[0] = xmin;
	ms->min[1] = ymin;
	ms->min[2] = zmin;
	ms->max[0] = xmax;
	ms->max[1] = ymax;
	ms->max[2] = zmax;
}

void msurf_get_bounds(struct metasurface *ms, float *xmin, float *ymin, float *zmin, float *xmax, float *ymax, float *zmax)
{
	*xmin = ms->min[0];
	*ymin = ms->min[1];
	*zmin = ms->min[2];
	*xmax = ms->max[0];
	*ymax = ms->max[1];
	*zmax = ms->max[2];
}

void msurf_set_resolution(struct metasurface *ms, int xres, int yres, int zres)
{
	ms->newres[0] = xres;
	ms->newres[1] = yres;
	ms->newres[2] = zres;
}

void msurf_get_resolution(struct metasurface *ms, int *xres, int *yres, int *zres)
{
	*xres = ms->res[0];
	*yres = ms->res[1];
	*zres = ms->res[2];
}

void msurf_set_threshold(struct metasurface *ms, float thres)
{
	ms->thres = thres;
}

float msurf_get_threshold(struct metasurface *ms)
{
	return ms->thres;
}


float *msurf_voxels(struct metasurface *ms)
{
	if(ms->res[0] != ms->newres[0] || ms->res[1] != ms->newres[1] || ms->res[2] != ms->newres[2]) {
		int count;
		ms->res[0] = ms->newres[0];
		ms->res[1] = ms->newres[1];
		ms->res[2] = ms->newres[2];
		count = ms->res[0] * ms->res[1] * ms->res[2];
		free(ms->voxels);
		if(!(ms->voxels = malloc(count * sizeof *ms->voxels))) {
			return 0;
		}
	}
	return ms->voxels;
}

float *msurf_slice(struct metasurface *ms, int idx)
{
	float *vox = msurf_voxels(ms);
	if(!vox) return 0;

	return vox + ms->res[0] * ms->res[1] * idx;
}

int msurf_polygonize(struct metasurface *ms)
{
	int i, j, k;
	vec3 pos, delta;

	if(!ms->voxels) return -1;

	ms->varr_size = 0;

	for(i=0; i<3; i++) {
		delta[i] = (ms->max[i] - ms->min[i]) / (float)ms->res[i];
	}

	for(i=0; i<ms->res[0] - 1; i++) {
		for(j=0; j<ms->res[1] - 1; j++) {
			for(k=0; k<ms->res[2] - 1; k++) {

				pos[0] = ms->min[0] + i * delta[0];
				pos[1] = ms->min[1] + j * delta[1];
				pos[2] = ms->min[2] + k * delta[2];

				process_cell(ms, i, j, k, pos, delta);
			}
		}
	}
	return 0;
}

int msurf_vertex_count(struct metasurface *ms)
{
	return ms->varr_size / 3;
}

float *msurf_vertices(struct metasurface *ms)
{
	return ms->varr;
}

float *msurf_normals(struct metasurface *ms)
{
	return ms->narr;
}

static unsigned int mc_bitcode(float *val, float thres);

extern int xyzzy;

static void process_cell(struct metasurface *ms, int xcell, int ycell, int zcell, vec3 cellpos, vec3 cellsz)
{
	int i, j, k, slice_size;
	vec3 pos[8];
	float dfdx[8], dfdy[8], dfdz[8];
	vec3 vert[12], norm[12];
	float val[8];
	float *cellptr;
	unsigned int code;

	static const int offs[][3] = {
		{0, 0, 0},
		{1, 0, 0},
		{1, 1, 0},
		{0, 1, 0},
		{0, 0, 1},
		{1, 0, 1},
		{1, 1, 1},
		{0, 1, 1}
	};

	static const int pidx[12][2] = {
		{0, 1}, {1, 2}, {2, 3}, {3, 0}, {4, 5}, {5, 6},
		{6, 7},	{7, 4}, {0, 4}, {1, 5}, {2, 6}, {3, 7}
	};

	slice_size = ms->res[0] * ms->res[1];
	cellptr = ms->voxels + slice_size * zcell + ms->res[0] * ycell + xcell;

#define GRIDOFFS(x, y, z)	((z) * slice_size + (y) * ms->res[0] + (x))

	for(i=0; i<8; i++) {
		val[i] = cellptr[GRIDOFFS(offs[i][0], offs[i][1], offs[i][2])];
	}

	code = mc_bitcode(val, ms->thres);
	if(ms->flags & MSURF_FLIP) {
		code = ~code & 0xff;
	}
	if(mc_edge_table[code] == 0) {
		return;
	}

	/* calculate normals at the 8 corners */
	for(i=0; i<8; i++) {
		float *ptr = cellptr + GRIDOFFS(offs[i][0], offs[i][1], offs[i][2]);

		if(xcell < ms->res[0] - 1) {
			dfdx[i] = ptr[GRIDOFFS(1, 0, 0)] - *ptr;
		} else {
			dfdx[i] = *ptr - ptr[GRIDOFFS(-1, 0, 0)];
		}
		if(ycell < ms->res[1] - 1) {
			dfdy[i] = ptr[GRIDOFFS(0, 1, 0)] - *ptr;
		} else {
			dfdy[i] = *ptr - ptr[GRIDOFFS(0, -1, 0)];
		}
		if(zcell < ms->res[2] - 1) {
			dfdz[i] = ptr[GRIDOFFS(0, 0, 1)] - *ptr;
		} else {
			dfdz[i] = *ptr - ptr[GRIDOFFS(0, 0, -1)];
		}
	}

	/* calculate the world-space position of each corner */
	for(i=0; i<8; i++) {
		pos[i][0] = cellpos[0] + cellsz[0] * offs[i][0];
		pos[i][1] = cellpos[1] + cellsz[1] * offs[i][1];
		pos[i][2] = cellpos[2] + cellsz[2] * offs[i][2];
	}

	/* generate up to a max of 12 vertices per cube. interpolate positions and normals for each one */
	for(i=0; i<12; i++) {
		if(mc_edge_table[code] & (1 << i)) {
			float nx, ny, nz;
			int p0 = pidx[i][0];
			int p1 = pidx[i][1];

			float t = (ms->thres - val[p0]) / (val[p1] - val[p0]);
			vert[i][0] = pos[p0][0] + (pos[p1][0] - pos[p0][0]) * t;
			vert[i][1] = pos[p0][1] + (pos[p1][1] - pos[p0][1]) * t;
			vert[i][2] = pos[p0][2] + (pos[p1][2] - pos[p0][2]) * t;

			nx = dfdx[p0] + (dfdx[p1] - dfdx[p0]) * t;
			ny = dfdy[p0] + (dfdy[p1] - dfdy[p0]) * t;
			nz = dfdz[p0] + (dfdz[p1] - dfdz[p0]) * t;

			if(ms->flags & MSURF_FLIP) {
				nx = -nx;
				ny = -ny;
				nz = -nz;
			}

			if(ms->flags & MSURF_NORMALIZE) {
				float s = rsqrt(nx * nx + ny * ny + nz * nz);
				nx *= s;
				ny *= s;
				nz *= s;
			}

			norm[i][0] = nx;
			norm[i][1] = ny;
			norm[i][2] = nz;
		}
	}

	/* for each triangle of the cube add the appropriate vertices to the vertex buffer */
	for(i=0; mc_tri_table[code][i] != -1; i+=3) {
		for(j=0; j<3; j++) {
			int idx = mc_tri_table[code][i + j];
			float *v = vert[idx];
			float *n = norm[idx];

			/* TODO multithreadied polygon emit */
			if(ms->varr_size + 3 > ms->varr_alloc_size) {
				int newsz = ms->varr_alloc_size ? ms->varr_alloc_size * 2 : 32768;
				float *new_varr, *new_narr;

				if(!(new_varr = realloc(ms->varr, newsz * sizeof *new_varr)) ||
						!(new_narr = realloc(ms->narr, newsz * sizeof *new_narr))) {
					free(new_varr);
					fprintf(stderr, "msurf_polygonize: failed to grow vertex buffers to %d elements\n", newsz);
					return;
				}
				ms->varr = new_varr;
				ms->narr = new_narr;
				ms->varr_alloc_size = newsz;
			}

			for(k=0; k<3; k++) {
				ms->varr[ms->varr_size] = v[k];
				ms->narr[ms->varr_size] = n[k];
				++ms->varr_size;
			}
		}
	}
}

static unsigned int mc_bitcode(float *val, float thres)
{
	unsigned int i, res = 0;

	for(i=0; i<8; i++) {
		if(val[i] > thres) {
			res |= 1 << i;
		}
	}
	return res;
}
