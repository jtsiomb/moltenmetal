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
