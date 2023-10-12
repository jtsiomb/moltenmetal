#include <stdio.h>
#include <string.h>
#include "cgmath/cgmath.h"
#include "game.h"
#include "metasurf.h"
#include "metaobj.h"
#include "util.h"

static struct mobject *mobj_create(int num);
static void swstate(struct mobject *mobj, int st);
static void update(struct mobject *mobj, float tsec);
static float eval(struct mobject *mobj, cgm_vec3 *pos);

static void upd_sflake_ball(struct mobject *mobj, struct mball *ball, float tsec, float t);
static int calc_num_balls(int depth);
static int gen_sflake(cgm_vec4 *sarr, int num, int depth, float x, float y, float z, float rad);

static void upd_sgi_caps(struct mobject *mobj, struct mcapsule *caps, float tsec, float t);

static float capsule_distsq(struct mcapsule *c, cgm_vec3 *pos);
static float easein(float x);
static float easeout(float x);


static struct mobject *mobj_create(int num)
{
	int i;
	struct mobject *mobj;

	mobj = calloc_nf(1, sizeof *mobj);

	mobj->idlepos = malloc_nf(num * sizeof *mobj->idlepos);
	mobj->mot = malloc_nf(num * sizeof *mobj->mot);

	for(i=0; i<num; i++) {
		mobj->mot[i].x = 2.0f * ((float)rand() / (float)RAND_MAX) - 1.0f;
		mobj->mot[i].y = 2.0f * ((float)rand() / (float)RAND_MAX) - 1.0f;
		mobj->mot[i].z = 2.0f * ((float)rand() / (float)RAND_MAX) - 1.0f;
		mobj->mot[i].w = 0.8;
	}

	mobj->swstate = swstate;
	mobj->update = update;
	mobj->eval = eval;

	mobj->state = -1;
	swstate(mobj, MOBJ_IDLE);
	return mobj;
}

static void swstate(struct mobject *mobj, int st)
{
	if(st == mobj->state) return;
	if(st == MOBJ_GRABING && mobj->state != MOBJ_IDLE) return;
	if(st == MOBJ_DROPPING && mobj->state != MOBJ_HELD && mobj->state != MOBJ_GRABING) {
		return;
	}

	switch(st) {
	case MOBJ_GRABING:
		if(mobj->state != MOBJ_IDLE) return;
		break;

	case MOBJ_DROPPING:
		if(mobj->state != MOBJ_HELD && mobj->state != MOBJ_GRABING) {
			return;
		}
		break;

	case MOBJ_IDLE:
		mobj->pos.x = mobj->pos.z = 0.0f;
		mobj->pos.y = -BBOX_YSZ * 0.5f;
		break;
	}

	mobj->state = st;
	mobj->tstart = (float)time_msec / 1000.0f;
}

static void update(struct mobject *mobj, float tsec)
{
	int i, count;
	struct mball *ball;
	struct mcapsule *caps;
	float t;
	cgm_vec3 *idleptr;
	cgm_vec4 *motptr;

	count = mobj->num_balls + mobj->num_caps;

	if(mobj->state != MOBJ_IDLE) {
		cgm_midentity(mobj->xform);
		cgm_mrotate_x(mobj->xform, tsec);
		cgm_mrotate_y(mobj->xform, tsec);
		cgm_mtranslate(mobj->xform, mobj->pos.x, mobj->pos.y, mobj->pos.z);
	}
	if(mobj->state != MOBJ_HELD) {
		for(i=0; i<count; i++) {
			mobj->idlepos[i].x = sin(tsec * mobj->mot[i].x + mobj->mot[i].y) * mobj->mot[i].z * 4.0f;
			mobj->idlepos[i].z = cos(tsec * mobj->mot[i].z + mobj->mot[i].y) * mobj->mot[i].x * 4.0f;
			mobj->idlepos[i].y = -BBOX_YSZ * 0.45f;
		}
	}

	idleptr = mobj->idlepos;
	motptr = mobj->mot;
	ball = mobj->balls;
	caps = mobj->caps;

	switch(mobj->state) {
	case MOBJ_IDLE:
		if(mobj->balls) {
			for(i=0; i<mobj->num_balls; i++) {
				ball->pos = idleptr[i];
				ball->energy = motptr[i].w;
				ball++;
			}
			idleptr += mobj->num_balls;
			motptr += mobj->num_balls;
		}
		if(mobj->caps) {
			for(i=0; i<mobj->num_caps; i++) {
				caps->end[0] = caps->end[1] = idleptr[i];
				caps->energy = motptr[i].w;
				caps++;
			}
		}
		break;

	case MOBJ_GRABING:
		t = easeout((tsec - mobj->tstart) / TRANSDUR);
		if(t >= 1.0f) mobj->swstate(mobj, MOBJ_HELD);
		if(0) {
	case MOBJ_DROPPING:
			t = easein((tsec - mobj->tstart) / TRANSDUR);
			if(t >= 1.0f) mobj->swstate(mobj, MOBJ_IDLE);
		}
		for(i=0; i<mobj->num_balls; i++) {
			mobj->upd_ball(mobj, ball++, tsec, t);
		}
		for(i=0; i<mobj->num_caps; i++) {
			mobj->upd_caps(mobj, caps++, tsec, t);
		}
		break;

	case MOBJ_HELD:
		for(i=0; i<mobj->num_balls; i++) {
			mobj->upd_ball(mobj, ball++, tsec, 0);
		}
		for(i=0; i<mobj->num_caps; i++) {
			mobj->upd_caps(mobj, caps++, tsec, 0);
		}
		break;
	}
}

static float eval(struct mobject *mobj, cgm_vec3 *pos)
{
	int i;
	float dsq, energy = 0.0f;
	struct mball *ball = mobj->balls;
	struct mcapsule *caps = mobj->caps;

	for(i=0; i<mobj->num_balls; i++) {
		dsq = cgm_vdist_sq(&ball->pos, pos);
		energy += ball->energy / dsq;
		ball++;
	}

	for(i=0; i<mobj->num_caps; i++) {
		dsq = capsule_distsq(mobj->caps + i, pos);
		energy += caps->energy / dsq;
	}
	return energy;
}


/* ---- sphereflake ---- */
#define SF_MAX_DEPTH	2
static cgm_vec4 *sfsph;

struct mobject *metaobj_sflake(void)
{
	int num_balls;
	struct mobject *mobj;

	num_balls = calc_num_balls(SF_MAX_DEPTH);

	mobj = mobj_create(num_balls);

	mobj->num_balls = num_balls;
	mobj->balls = malloc_nf(num_balls * sizeof *mobj->balls);
	sfsph = malloc_nf(num_balls * sizeof *sfsph);

	gen_sflake(sfsph, 0, SF_MAX_DEPTH, 0, 0, 0, 20);

	mobj->upd_ball = upd_sflake_ball;
	return mobj;
}

static void upd_sflake_ball(struct mobject *mobj, struct mball *ball, float tsec, float t)
{
	int idx = ball - mobj->balls;
	cgm_vec3 pos;

	switch(mobj->state) {
	case MOBJ_DROPPING:
		t = 1.0f - t;
	case MOBJ_GRABING:
		cgm_vcons(&pos, sfsph[idx].x, sfsph[idx].y, sfsph[idx].z);
		cgm_vmul_m4v3(&pos, mobj->xform);
		cgm_vlerp(&ball->pos, mobj->idlepos + idx, &pos, t);
		ball->energy = cgm_lerp(mobj->mot[idx].w, sfsph[idx].w, t);
		break;

	case MOBJ_HELD:
		cgm_vcons(&ball->pos, sfsph[idx].x, sfsph[idx].y, sfsph[idx].z);
		cgm_vmul_m4v3(&ball->pos, mobj->xform);
		ball->energy = sfsph[idx].w;
		break;
	}
}

static int calc_num_balls(int depth)
{
	if(!depth) return 0;
	return calc_num_balls(depth - 1) * 6 + 1;
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

	cgm_midentity(sgimat);
	cgm_mrotate_y(sgimat, -M_PI / 4.0f);
	cgm_mrotate_x(sgimat, M_PI / 4.0f);
	cgm_mtranslate(sgimat, 0, -4, 0);

	mobj = mobj_create(NUM_SGI_VERTS);

	mobj->num_caps = NUM_SGI_VERTS;
	mobj->caps = calloc_nf(mobj->num_caps, sizeof *mobj->caps);

	for(i=0; i<mobj->num_caps; i++) {
		mobj->caps[i].energy = 0.7;
	}

	mobj->swstate = swstate;
	mobj->upd_caps = upd_sgi_caps;
	return mobj;
}

#define LOGOSCALE	0.55f
static void upd_sgi_caps(struct mobject *mobj, struct mcapsule *caps, float tsec, float t)
{
	int idx0, idx1;
	cgm_vec3 pos[2];
	static cgm_vec3 prev_pos;

	idx0 = caps - mobj->caps;
	idx1 = idx0 >= mobj->num_caps - 1 ? 0 : idx0 + 1;

	switch(mobj->state) {
	case MOBJ_DROPPING:
		t = 1.0f - t;
	case MOBJ_GRABING:
		if(idx0 == 0) {
			pos[0] = sgiv[idx0];
			cgm_vscale(pos, LOGOSCALE);
			cgm_vmul_m4v3(pos, mobj->xform);
			cgm_vlerp(caps->end, mobj->idlepos + idx0, pos, t);
		} else {
			caps->end[0] = prev_pos;
		}
		pos[1] = sgiv[idx1];
		cgm_vscale(pos + 1, LOGOSCALE);
		cgm_vmul_m4v3(pos + 1, mobj->xform);
		cgm_vlerp(caps->end + 1, mobj->idlepos + idx1, pos + 1, t);
		prev_pos = caps->end[1];
		/*caps->energy = cgm_lerp(mobj->mot[idx].w, sfsph[idx].w, t);*/
		break;

	case MOBJ_HELD:
		if(idx0 == 0) {
			pos[0] = sgiv[0];
			cgm_vscale(pos, LOGOSCALE);
			cgm_vmul_m4v3(pos, mobj->xform);
			caps->end[0] = pos[0];
		} else {
			caps->end[0] = prev_pos;
		}
		pos[1] = sgiv[idx1];
		cgm_vscale(pos + 1, LOGOSCALE);
		cgm_vmul_m4v3(pos + 1, mobj->xform);
		prev_pos = caps->end[1] = pos[1];
		break;
	}
	caps->len = cgm_vdist(caps->end, caps->end + 1);
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

static float easein(float x)
{
	return x * x * x;
}

static float easeout(float x)
{
	return 1.0f - pow(1.0f - x, 3.0f);
}
