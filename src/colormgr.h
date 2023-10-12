#ifndef COLORMGR_H_
#define COLORMGR_H_

#define SHADE_LEVELS	8
#define SHADE_SHIFT		3

#define LOOKUP_SHADE(col, shade) \
	shade_lut[(col << SHADE_SHIFT) + ((255 - (shade)) >> (8 - SHADE_SHIFT))]

extern unsigned char *colormap;
extern int *shade_lut;

void init_colormgr(void);

void load_colormap(int offs, int sz, unsigned char *col, unsigned char *slut);

int find_color(int r, int g, int b);
int shade_color(int col, int shade);	/* both 0-255 */

#endif	/* COLORMGR_H_ */
