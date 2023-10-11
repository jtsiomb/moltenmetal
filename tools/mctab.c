#include <stdio.h>
#include "mcubes.h"

int main(void)
{
	int i, j, len, n;
	unsigned char byte;

	printf("static unsigned char tritab_runlen[128] = {\n\t");

	for(i=0; i<128; i++) {
		unsigned int val;
		for(j=0; j<16; j++) {
			if(mc_tri_table[i * 2][j] == -1) break;
		}
		val = j;
		for(j=0; j<16; j++) {
			if(mc_tri_table[i * 2 + 1][j] == -1) break;
		}
		val |= j << 4;
		printf("0x%02x", val);
		if(i == 127) {
			printf("\n};\n");
		} else if(i % 12 != 11) {
			printf(", ");
		} else {
			printf(",\n\t");
		}
	}

	printf("\nstatic unsigned char tritab_data[] = {\n\t");

	len = 4;
	byte = 0;
	n = 0;
	for(i=0; i<256; i++) {
		for(j=0; j<16; j++) {
			if(mc_tri_table[i][j] == -1) break;

			if(n++ == 0) {
				byte = mc_tri_table[i][j];
			} else {
				byte |= mc_tri_table[i][j] << 4;
				n = 0;

				len += printf("%d", byte);
				if(i < 254 || (j < 15 && mc_tri_table[i][j + 1] != -1)) {
					if(len < 72) {
						len += printf(", ");
					} else {
						printf(",\n\t");
						len = 4;
					}
				}
			}
		}
	}
	printf("\n};\n");

	return 0;
}
