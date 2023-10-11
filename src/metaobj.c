#include <stdio.h>
#include <string.h>
#include "cgmath/cgmath.h"
#include "metasurf.h"
#include "metaobj.h"
#include "util.h"

static void upd_sflake(struct mobject *mobj, float t);
static float eval_sflake(struct mobject *mobj, cgm_vec3 *pos);
static int num_balls(int depth);
static int gen_sflake(cgm_vec4 *sarr, int num, int depth, float x, float y, float z, float rad);

static void upd_sgi(struct mobject *mobj, float t);
static float eval_sgi(struct mobject *mobj, cgm_vec3 *pos);
static float capsule_distsq(struct mcapsule *c, cgm_vec3 *pos);

/* ---- sphereflake ---- */
#define SF_MAX_DEPTH	2
static cgm_vec4 *sfsph;

struct mobject *metaobj_sflake(void)
{
	struct mobject *mobj;

	mobj = calloc_nf(1, sizeof *mobj);

	mobj->num_balls = num_balls(SF_MAX_DEPTH);
	mobj->balls = malloc_nf(mobj->num_balls * sizeof *mobj->balls);
	sfsph = malloc_nf(mobj->num_balls * sizeof *sfsph);

	gen_sflake(sfsph, 0, SF_MAX_DEPTH, 0, 0, 0, 20);

	mobj->update = upd_sflake;
	mobj->eval = eval_sflake;
	return mobj;
}

static void upd_sflake(struct mobject *mobj, float t)
{
	int i;
	struct mball *ball = mobj->balls;
	float mat[16];

	cgm_midentity(mat);
	cgm_mrotate_x(mat, t);
	cgm_mrotate_y(mat, t);
	cgm_mtranslate(mat, mobj->pos.x, mobj->pos.y, mobj->pos.z);

	for(i=0; i<mobj->num_balls; i++) {
		cgm_vcons(&ball->pos, sfsph[i].x, sfsph[i].y, sfsph[i].z);
		cgm_vmul_m4v3(&ball->pos, mat);
		ball->energy = sfsph[i].w;
		ball++;
	}
}

static float eval_sflake(struct mobject *mobj, cgm_vec3 *pos)
{
	int i;
	float dsq, energy = 0.0f;
	struct mball *ball = mobj->balls;

	for(i=0; i<mobj->num_balls; i++) {
		dsq = cgm_vdist_sq(&ball->pos, pos);
		energy += ball->energy / dsq;
		ball++;
	}
	return energy;
}

static int num_balls(int depth)
{
	if(!depth) return 0;
	return num_balls(depth - 1) * 6 + 1;
}

static int gen_sflake(cgm_vec4 *sarr, int num, int depth, float x, float y, float z, float rad)
{
	int subnum;
	float subrad, offs;

	if(!depth) return 0;

	sarr[num].x = x;
	sarr[num].y = y;
	sarr[num].z = z;
	sarr[num].w = rad;
	num++;

	subrad = rad * 0.2f;
	offs = rad * 0.16f;

	subnum = 0;
	subnum += gen_sflake(sarr, num + subnum, depth - 1, x + offs, y, z, subrad);
	subnum += gen_sflake(sarr, num + subnum, depth - 1, x - offs, y, z, subrad);
	subnum += gen_sflake(sarr, num + subnum, depth - 1, x, y + offs, z, subrad);
	subnum += gen_sflake(sarr, num + subnum, depth - 1, x, y - offs, z, subrad);
	subnum += gen_sflake(sarr, num + subnum, depth - 1, x, y, z + offs, subrad);
	subnum += gen_sflake(sarr, num + subnum, depth - 1, x, y, z - offs, subrad);
	return subnum + 1;
}

/* ---- SGI logo ---- */

static const cgm_vec3 sgiv[] = {
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
#define NUM_SGI_VERTS	(sizeof sgiv / sizeof *sgiv)
static float sgimat[16];

struct mobject *metaobj_sgi(void)
{
	int i;
	struct mobject *mobj;

	mobj = calloc_nf(1, sizeof *mobj);

	cgm_midentity(sgimat);
	cgm_mrotate_y(sgimat, -M_PI / 4.0f);
	cgm_mrotate_x(sgimat, M_PI / 4.0f);
	cgm_mtranslate(sgimat, 0, -4, 0);

	mobj->num_caps = NUM_SGI_VERTS;
	mobj->caps = calloc_nf(mobj->num_caps, sizeof *mobj->caps);

	for(i=0; i<mobj->num_caps; i++) {
		mobj->caps[i].energy = 0.7;
	}

	mobj->update = upd_sgi;
	mobj->eval = eval_sgi;
	return mobj;
}

static void upd_sgi(struct mobject *mobj, float t)
{
	int i;
	float mat[16];
	cgm_vec3 vpos[NUM_SGI_VERTS];

	cgm_mcopy(mat, sgimat);
	cgm_mrotate_y(mat, t);
	cgm_mtranslate(mat, mobj->pos.x, mobj->pos.y, mobj->pos.z);

	for(i=0; i<NUM_SGI_VERTS; i++) {
		vpos[i] = sgiv[i];
		cgm_vscale(vpos + i, 0.5);
		cgm_vmul_m4v3(vpos + i, mat);
	}

	for(i=0; i<NUM_SGI_VERTS; i++) {
		mobj->caps[i].end[0] = vpos[i];
		mobj->caps[i].end[1] = vpos[(i + 1) % NUM_SGI_VERTS];
		mobj->caps[i].len = cgm_vdist(mobj->caps[i].end, mobj->caps[i].end + 1);
	}
}

static float eval_sgi(struct mobject *mobj, cgm_vec3 *pos)
{
	int i;
	float dsq, val = 0.0f;

	for(i=0; i<mobj->num_caps; i++) {
		dsq = capsule_distsq(mobj->caps + i, pos);
		val += mobj->caps[i].energy / dsq;
	}

	return val;
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
