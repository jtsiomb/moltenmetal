#ifndef COLORMGR_H_
#define COLORMGR_H_

void init_colormgr(void);

int find_color(int r, int g, int b);
int shade_color(int col, int shade);	/* both 0-255 */

#endif	/* COLORMGR_H_ */
