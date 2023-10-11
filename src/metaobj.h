#ifndef METAOBJ_H_
#define METAOBJ_H_


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
	cgm_vec3 pos;
	struct mball *balls;
	struct mcapsule *caps;
	int num_balls, num_caps;

	void (*update)(struct mobject *mobj, float t);
	float (*eval)(struct mobject *mobj, cgm_vec3 *pos);
};

struct mobject *metaobj_sflake(void);
struct mobject *metaobj_sgi(void);

#endif	/* METAOBJ_H_ */
