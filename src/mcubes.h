static short mc_edge_table[256] = {
	0x0  , 0x109, 0x203, 0x30a, 0x406, 0x50f, 0x605, 0x70c,
	0x80c, 0x905, 0xa0f, 0xb06, 0xc0a, 0xd03, 0xe09, 0xf00,
	0x190, 0x99 , 0x393, 0x29a, 0x596, 0x49f, 0x795, 0x69c,
	0x99c, 0x895, 0xb9f, 0xa96, 0xd9a, 0xc93, 0xf99, 0xe90,
	0x230, 0x339, 0x33 , 0x13a, 0x636, 0x73f, 0x435, 0x53c,
	0xa3c, 0xb35, 0x83f, 0x936, 0xe3a, 0xf33, 0xc39, 0xd30,
	0x3a0, 0x2a9, 0x1a3, 0xaa , 0x7a6, 0x6af, 0x5a5, 0x4ac,
	0xbac, 0xaa5, 0x9af, 0x8a6, 0xfaa, 0xea3, 0xda9, 0xca0,
	0x460, 0x569, 0x663, 0x76a, 0x66 , 0x16f, 0x265, 0x36c,
	0xc6c, 0xd65, 0xe6f, 0xf66, 0x86a, 0x963, 0xa69, 0xb60,
	0x5f0, 0x4f9, 0x7f3, 0x6fa, 0x1f6, 0xff , 0x3f5, 0x2fc,
	0xdfc, 0xcf5, 0xfff, 0xef6, 0x9fa, 0x8f3, 0xbf9, 0xaf0,
	0x650, 0x759, 0x453, 0x55a, 0x256, 0x35f, 0x55 , 0x15c,
	0xe5c, 0xf55, 0xc5f, 0xd56, 0xa5a, 0xb53, 0x859, 0x950,
	0x7c0, 0x6c9, 0x5c3, 0x4ca, 0x3c6, 0x2cf, 0x1c5, 0xcc ,
	0xfcc, 0xec5, 0xdcf, 0xcc6, 0xbca, 0xac3, 0x9c9, 0x8c0,
	0x8c0, 0x9c9, 0xac3, 0xbca, 0xcc6, 0xdcf, 0xec5, 0xfcc,
	0xcc , 0x1c5, 0x2cf, 0x3c6, 0x4ca, 0x5c3, 0x6c9, 0x7c0,
	0x950, 0x859, 0xb53, 0xa5a, 0xd56, 0xc5f, 0xf55, 0xe5c,
	0x15c, 0x55 , 0x35f, 0x256, 0x55a, 0x453, 0x759, 0x650,
	0xaf0, 0xbf9, 0x8f3, 0x9fa, 0xef6, 0xfff, 0xcf5, 0xdfc,
	0x2fc, 0x3f5, 0xff , 0x1f6, 0x6fa, 0x7f3, 0x4f9, 0x5f0,
	0xb60, 0xa69, 0x963, 0x86a, 0xf66, 0xe6f, 0xd65, 0xc6c,
	0x36c, 0x265, 0x16f, 0x66 , 0x76a, 0x663, 0x569, 0x460,
	0xca0, 0xda9, 0xea3, 0xfaa, 0x8a6, 0x9af, 0xaa5, 0xbac,
	0x4ac, 0x5a5, 0x6af, 0x7a6, 0xaa , 0x1a3, 0x2a9, 0x3a0,
	0xd30, 0xc39, 0xf33, 0xe3a, 0x936, 0x83f, 0xb35, 0xa3c,
	0x53c, 0x435, 0x73f, 0x636, 0x13a, 0x33 , 0x339, 0x230,
	0xe90, 0xf99, 0xc93, 0xd9a, 0xa96, 0xb9f, 0x895, 0x99c,
	0x69c, 0x795, 0x49f, 0x596, 0x29a, 0x393, 0x99 , 0x190,
	0xf00, 0xe09, 0xd03, 0xc0a, 0xb06, 0xa0f, 0x905, 0x80c,
	0x70c, 0x605, 0x50f, 0x406, 0x30a, 0x203, 0x109, 0x0
};

static unsigned char tritab_runlen[128] = {
	0x30, 0x63, 0x63, 0x96, 0x63, 0x96, 0x96, 0x69, 0x63, 0x96, 0x96, 0xc9,
	0x96, 0xc9, 0xc9, 0x9c, 0x63, 0x96, 0x96, 0xc9, 0x96, 0xc9, 0xc9, 0x9c,
	0x96, 0x69, 0xc9, 0x9c, 0xc9, 0x9c, 0xfc, 0x6f, 0x63, 0x96, 0x96, 0xc9,
	0x96, 0xc9, 0xc9, 0x9c, 0x96, 0xc9, 0xc9, 0xfc, 0xc9, 0xfc, 0xfc, 0xcf,
	0x96, 0xc9, 0xc9, 0x96, 0xc9, 0xfc, 0xfc, 0x69, 0xc9, 0x9c, 0xfc, 0x69,
	0xfc, 0xcf, 0x6f, 0x3c, 0x63, 0x96, 0x96, 0xc9, 0x96, 0xc9, 0xc9, 0x9c,
	0x96, 0xc9, 0xc9, 0xfc, 0x69, 0x9c, 0x9c, 0x6f, 0x96, 0xc9, 0xc9, 0xfc,
	0xc9, 0xfc, 0xfc, 0xcf, 0xc9, 0x9c, 0xfc, 0xcf, 0x9c, 0x6f, 0xcf, 0x36,
	0x96, 0xc9, 0xc9, 0xfc, 0xc9, 0xfc, 0x96, 0x69, 0xc9, 0xfc, 0xfc, 0x6f,
	0x9c, 0xcf, 0x69, 0x3c, 0xc9, 0xfc, 0xfc, 0xc9, 0xfc, 0x6f, 0xc9, 0x36,
	0x96, 0x69, 0xc9, 0x36, 0x69, 0x3c, 0x36, 0x03
};

static unsigned char tritab_data[] = {
	128, 3, 145, 129, 147, 24, 33, 10, 56, 33, 154, 162, 32, 41, 56, 162,
	168, 137, 179, 2, 43, 184, 16, 9, 50, 27, 43, 145, 155, 184, 163, 177,
	58, 160, 1, 168, 184, 58, 9, 179, 185, 154, 137, 170, 184, 116, 72, 3,
	55, 4, 145, 72, 71, 145, 116, 113, 19, 33, 138, 116, 67, 55, 64, 33, 154,
	162, 9, 130, 116, 162, 41, 121, 114, 115, 73, 72, 55, 43, 75, 183, 66,
	2, 148, 16, 72, 39, 179, 116, 155, 180, 185, 146, 18, 163, 49, 171, 135,
	20, 171, 65, 27, 64, 183, 68, 135, 9, 155, 171, 11, 67, 183, 180, 153,
	171, 89, 148, 69, 128, 3, 69, 81, 128, 69, 56, 53, 81, 33, 154, 69, 3,
	24, 162, 148, 85, 162, 69, 66, 32, 162, 53, 82, 83, 52, 132, 89, 36, 179,
	176, 2, 184, 148, 5, 69, 16, 37, 179, 18, 37, 133, 130, 75, 88, 58, 171,
	49, 89, 68, 89, 128, 129, 26, 184, 90, 4, 5, 91, 171, 11, 83, 132, 133,
	170, 184, 121, 88, 151, 57, 144, 53, 117, 3, 135, 16, 23, 117, 81, 51,
	117, 121, 152, 117, 26, 162, 33, 89, 80, 3, 117, 131, 32, 40, 133, 117,
	90, 34, 90, 82, 51, 117, 151, 117, 152, 179, 146, 117, 121, 146, 2, 114,
	43, 179, 16, 24, 135, 81, 183, 18, 27, 119, 81, 89, 136, 117, 26, 163,
	179, 117, 80, 144, 183, 16, 160, 171, 176, 10, 11, 163, 5, 8, 87, 7, 171,
	117, 91, 106, 5, 56, 165, 150, 16, 165, 22, 56, 145, 88, 106, 97, 37,
	22, 97, 21, 98, 3, 152, 86, 9, 6, 98, 149, 88, 40, 37, 54, 130, 50, 171,
	86, 11, 184, 2, 106, 5, 145, 50, 91, 106, 165, 22, 41, 185, 146, 184,
	54, 107, 53, 21, 3, 184, 176, 5, 21, 181, 54, 107, 48, 6, 86, 80, 105,
	149, 150, 187, 137, 165, 70, 135, 52, 64, 55, 86, 26, 9, 165, 134, 116,
	106, 21, 121, 113, 115, 73, 22, 98, 21, 116, 24, 82, 37, 54, 64, 67, 135,
	116, 9, 5, 86, 32, 118, 147, 151, 52, 146, 149, 38, 150, 179, 114, 72,
	106, 85, 106, 116, 66, 2, 114, 11, 145, 116, 40, 179, 165, 150, 18, 185,
	146, 180, 183, 84, 106, 72, 55, 91, 83, 81, 107, 21, 91, 107, 1, 123,
	75, 64, 11, 149, 96, 5, 99, 107, 131, 116, 86, 105, 185, 116, 121, 155,
	74, 105, 164, 164, 70, 169, 128, 163, 16, 106, 96, 4, 56, 129, 97, 104,
	100, 161, 65, 25, 66, 98, 52, 128, 33, 41, 148, 98, 4, 66, 36, 134, 35,
	40, 68, 98, 74, 169, 70, 43, 3, 40, 130, 75, 169, 164, 54, 43, 16, 6,
	70, 22, 106, 20, 22, 74, 24, 18, 139, 27, 105, 148, 99, 25, 179, 54, 184,
	129, 1, 107, 145, 65, 70, 49, 107, 99, 0, 70, 70, 184, 134, 167, 118,
	168, 152, 10, 55, 160, 7, 169, 118, 170, 118, 161, 23, 135, 129, 160,
	118, 122, 17, 55, 33, 22, 134, 129, 137, 118, 98, 41, 25, 118, 9, 57,
	55, 121, 8, 7, 102, 32, 55, 98, 39, 50, 171, 134, 138, 137, 118, 2, 39,
	183, 144, 103, 167, 169, 23, 8, 113, 24, 122, 118, 42, 179, 43, 177, 113,
	106, 97, 23, 152, 134, 118, 25, 182, 54, 49, 6, 25, 107, 119, 8, 7, 54,
	11, 107, 112, 107, 103, 59, 128, 123, 6, 145, 123, 134, 145, 56, 177,
	103, 26, 98, 123, 33, 58, 128, 182, 39, 9, 162, 105, 123, 182, 39, 58,
	138, 163, 137, 39, 99, 114, 7, 120, 6, 38, 32, 103, 50, 7, 145, 97, 18,
	104, 145, 136, 103, 122, 166, 113, 49, 167, 103, 113, 26, 120, 1, 8, 115,
	112, 10, 154, 166, 119, 166, 167, 136, 154, 134, 180, 104, 99, 59, 96,
	64, 134, 182, 72, 150, 16, 73, 150, 54, 57, 177, 99, 134, 100, 139, 162,
	17, 162, 3, 11, 182, 64, 70, 139, 100, 11, 146, 162, 169, 57, 58, 146,
	52, 59, 70, 54, 40, 131, 36, 100, 2, 36, 100, 18, 9, 50, 36, 100, 52,
	24, 73, 65, 34, 100, 24, 131, 22, 72, 102, 26, 26, 160, 96, 6, 68, 54,
	52, 104, 58, 48, 169, 57, 154, 100, 74, 148, 117, 182, 128, 67, 89, 123,
	86, 16, 69, 112, 182, 123, 134, 67, 83, 52, 81, 89, 164, 33, 103, 107,
	123, 33, 10, 56, 148, 117, 182, 69, 74, 162, 4, 50, 132, 83, 52, 82, 90,
	178, 103, 39, 115, 38, 69, 153, 69, 128, 6, 38, 134, 55, 38, 115, 22,
	5, 69, 96, 130, 134, 39, 129, 132, 21, 133, 89, 164, 97, 113, 22, 115,
	97, 26, 103, 1, 135, 7, 89, 68, 160, 164, 5, 163, 166, 55, 167, 103, 122,
	138, 69, 74, 168, 150, 101, 155, 139, 57, 182, 96, 3, 101, 144, 5, 139,
	80, 11, 81, 101, 107, 59, 54, 85, 19, 33, 154, 181, 185, 184, 101, 176,
	3, 182, 144, 86, 150, 33, 186, 88, 91, 134, 80, 90, 2, 82, 182, 99, 83,
	162, 163, 53, 133, 89, 130, 101, 50, 40, 89, 150, 6, 96, 18, 133, 129,
	80, 134, 131, 98, 130, 81, 38, 97, 49, 22, 166, 131, 86, 150, 152, 166,
	1, 10, 150, 5, 101, 0, 131, 101, 170, 101, 91, 122, 181, 91, 186, 87,
	56, 80, 123, 165, 27, 9, 122, 165, 123, 137, 129, 19, 27, 178, 23, 87,
	1, 56, 33, 23, 87, 39, 155, 87, 41, 151, 32, 178, 119, 37, 39, 91, 41,
	35, 152, 40, 82, 42, 83, 115, 133, 2, 88, 130, 87, 42, 149, 16, 165, 83,
	115, 163, 146, 40, 41, 129, 39, 42, 117, 37, 49, 53, 87, 128, 7, 23, 113,
	149, 48, 57, 85, 115, 137, 87, 121, 133, 84, 138, 186, 88, 64, 181, 80,
	186, 59, 0, 145, 72, 138, 186, 74, 165, 75, 74, 181, 67, 73, 49, 65, 82,
	33, 88, 178, 72, 133, 64, 11, 59, 84, 43, 27, 21, 11, 82, 80, 41, 91,
	84, 184, 88, 73, 37, 59, 82, 58, 37, 67, 53, 72, 165, 82, 66, 36, 48,
	42, 83, 58, 88, 84, 8, 145, 165, 82, 66, 145, 146, 36, 72, 133, 53, 83,
	1, 84, 1, 133, 84, 88, 147, 80, 48, 149, 84, 180, 71, 185, 169, 11, 56,
	148, 151, 123, 169, 27, 186, 177, 20, 4, 71, 59, 65, 67, 24, 74, 71, 171,
	75, 180, 151, 75, 41, 155, 33, 121, 148, 123, 25, 43, 27, 128, 179, 71,
	75, 34, 4, 123, 180, 36, 56, 52, 66, 146, 42, 151, 50, 119, 148, 169,
	151, 71, 42, 135, 7, 2, 55, 167, 163, 114, 164, 161, 64, 160, 161, 130,
	71, 148, 65, 113, 23, 67, 25, 20, 7, 24, 120, 65, 48, 71, 67, 120, 169,
	168, 139, 3, 57, 185, 155, 10, 161, 160, 136, 186, 19, 186, 163, 33, 27,
	155, 185, 56, 144, 147, 27, 146, 178, 9, 178, 8, 59, 178, 50, 40, 168,
	138, 153, 42, 144, 34, 131, 130, 10, 129, 161, 24, 42, 49, 152, 129, 144,
	1, 131
};
