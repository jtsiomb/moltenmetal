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

	mesh = cmesh_alloc();
	if(cmesh_load(mesh, argv[1]) == -1) {
		fprintf(stderr, "failed to load mesh: %s\n", argv[1]);
		return 1;
	}

	if(!(fp = fopen(argv[2], "wb"))) {
		fprintf(stderr, "failed to open output file: %s: %s\n", argv[2], strerror(errno));
		return 1;
	}

	nverts = cmesh_attrib_count(mesh, CMESH_ATTR_VERTEX);
	ntri = cmesh_poly_count(mesh);
	nidx = cmesh_index_count(mesh);

	fwrite(&nverts, sizeof nverts, 1, fp);
	fwrite(&ntri, sizeof ntri, 1, fp);

	varr = cmesh_attrib_ro(mesh, CMESH_ATTR_VERTEX);
	fwrite(varr, 3 * sizeof *varr, nverts, fp);

	varr = cmesh_attrib_ro(mesh, CMESH_ATTR_TEXCOORD);
	fwrite(varr, 2 * sizeof(float), nverts, fp);

	idxarr = cmesh_index_ro(mesh);
	for(i=0; i<nidx; i++) {
		vidx = (uint16_t)idxarr[i];
		fwrite(&vidx, sizeof vidx, 1, fp);
	}

	fclose(fp);
	return 0;
}
