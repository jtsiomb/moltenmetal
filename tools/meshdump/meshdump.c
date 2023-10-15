#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include "cmesh.h"

struct cmesh *mesh;

int main(int argc, char **argv)
{
	int i, nidx;
	FILE *fp;
	uint16_t nverts, ntri;
	const float *varr;
	const unsigned int *idxarr;
	uint16_t vidx;
	float scale, inv_scale, rad;

	mesh = cmesh_alloc();
	if(cmesh_load(mesh, argv[1]) == -1) {
		fprintf(stderr, "failed to load mesh: %s\n", argv[1]);
		return 1;
	}
	rad = cmesh_bsphere(mesh, 0, 0);

	scale = 128.0 / rad;

	if(!(fp = fopen(argv[2], "wb"))) {
		fprintf(stderr, "failed to open output file: %s: %s\n", argv[2], strerror(errno));
		return 1;
	}

	nverts = cmesh_attrib_count(mesh, CMESH_ATTR_VERTEX);
	ntri = cmesh_poly_count(mesh);
	nidx = cmesh_index_count(mesh);

	inv_scale = 1.0f / scale;
	fwrite(&inv_scale, sizeof inv_scale, 1, fp);
	fwrite(&nverts, sizeof nverts, 1, fp);
	fwrite(&ntri, sizeof ntri, 1, fp);

	varr = cmesh_attrib_ro(mesh, CMESH_ATTR_VERTEX);
	for(i=0; i<nverts * 3; i++) {
		int16_t val = (int16_t)((varr[i] * scale) * 256.0f);
		fwrite(&val, sizeof val, 1, fp);
	}
	varr = cmesh_attrib_ro(mesh, CMESH_ATTR_TEXCOORD);
	for(i=0; i<nverts * 2; i++) {
		uint8_t val = (uint8_t)(varr[i] * 255.0f);
		fwrite(&val, 1, 1, fp);
	}

	idxarr = cmesh_index_ro(mesh);
	for(i=0; i<nidx; i++) {
		vidx = (uint16_t)idxarr[i];
		fwrite(&vidx, sizeof vidx, 1, fp);
	}

	fclose(fp);
	return 0;
}
