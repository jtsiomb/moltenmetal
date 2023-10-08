#ifndef GFXUTIL_H_
#define GFXUTIL_H_

#define PACK_RGB32(r, g, b) \
	((((r) & 0xff) << 16) | (((g) & 0xff) << 8) | ((b) & 0xff) | 0xff000000)

#define UNPACK_R32(c)	(((c) >> 16) & 0xff)
#define UNPACK_G32(c)	(((c) >> 8) & 0xff)
#define UNPACK_B32(c)	((c) & 0xff)


int clip_line(int *x0, int *y0, int *x1, int *y1, int xmin, int ymin, int xmax, int ymax);
void draw_line(int x0, int y0, int x1, int y1, unsigned char color);

void draw_billboard(float x, float y, float z, float size, int lum, int a);

#endif	/* GFXUTIL_H_ */
