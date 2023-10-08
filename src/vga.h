#ifndef VGA_H_
#define VGA_H_

void vga_setpal(int startidx, int count, unsigned char *cmap);
void vga_setpalent(int idx, int r, int g, int b);

#define wait_vsync() \
	asm volatile ( \
		"0: in $0x3da, %%al\n\t" \
		"jnz 0b\n\t" \
		"0: in $0x3da, %%al\n\t" \
		"jz 0b\n\t" \
		::: "eax")

#endif	/* VGA_H_ */
