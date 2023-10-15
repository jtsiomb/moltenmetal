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
#ifndef METAOBJ_H_
#define METAOBJ_H_

enum {
	MOBJ_IDLE,
	MOBJ_GRABING,
	MOBJ_HELD,
	MOBJ_DROPPING
};

struct mball {
	float energy;
	cgm_vec3 pos;
};

struct mcapsule {
	float energy;
	cgm_vec3 end[2];
	float len;
};

struct mobject {
	cgm_vec3 pos, mouse;
	int state;
	struct mball *balls;
	struct mcapsule *caps;
	int num_balls, num_caps;
	cgm_vec3 *idlepos;
	cgm_vec4 *mot;
	float tstart;
	float xform[16];

	void (*swstate)(struct mobject *mobj, int newst);
	void (*update)(struct mobject *mobj, float tsec);
	void (*upd_ball)(struct mobject *mobj, struct mball *ball, float tsec, float t);
	void (*upd_caps)(struct mobject *mobj, struct mcapsule *caps, float tsec, float t);
	float (*eval)(struct mobject *mobj, cgm_vec3 *pos);
};

struct mobject *metaobj_sflake(void);
struct mobject *metaobj_sgi(void);

#endif	/* METAOBJ_H_ */
