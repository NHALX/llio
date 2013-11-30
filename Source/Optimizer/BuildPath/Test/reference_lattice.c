#include "../lattice.h"
#include "reference_lattice.h"

///////////////////////// REFERENCE POSET 1: youmuu's ghostblade //////////////////

ideal_t reference_poset[5][2] = {
	{ 3, 1 },
	{ 5, 4 },
	{ 6, 4 },
	{ 1, 2 },
	{ 4, 2 }
};

t_edge reference_tree[15] = {
	{ { 0, 0 }, { 6, 1 }, 6 },
	{ { 6, 1 }, { 5, 2 }, 5 },
	{ { 5, 2 }, { 4, 3 }, 4 },
	{ { 4, 3 }, { 3, 4 }, 3 },
	{ { 3, 4 }, { 2, 5 }, 2 },
	{ { 2, 5 }, { 1, 6 }, 1 },
	{ { 4, 3 }, { 2, 7 }, 2 },
	{ { 2, 7 }, { 1, 8 }, 1 },
	{ { 5, 2 }, { 3, 11 }, 3 },
	{ { 3, 11 }, { 2, 12 }, 2 },
	{ { 2, 12 }, { 1, 13 }, 1 },
	{ { 5, 2 }, { 2, 9 }, 2 },
	{ { 2, 9 }, { 1, 10 }, 1 },
	{ { 6, 1 }, { 2, 14 }, 2 },
	{ { 2, 14 }, { 1, 15 }, 1 }
};

t_edge reference_lattice[26] = {
	{ { 0, 0 }, { 6, 1 }, 6 },
	{ { 6, 1 }, { 5, 2 }, 5 },
	{ { 5, 2 }, { 4, 3 }, 4 },
	{ { 4, 3 }, { 3, 4 }, 3 },
	{ { 3, 4 }, { 2, 5 }, 2 },
	{ { 2, 5 }, { 1, 6 }, 1 },
	{ { 4, 3 }, { 2, 7 }, 2 },
	{ { 2, 7 }, { 2, 5 }, 3 },
	{ { 2, 7 }, { 1, 8 }, 1 },
	{ { 1, 8 }, { 1, 6 }, 3 },
	{ { 5, 2 }, { 3, 11 }, 3 },
	{ { 3, 11 }, { 3, 4 }, 4 },
	{ { 3, 11 }, { 2, 12 }, 2 },
	{ { 2, 12 }, { 2, 5 }, 4 },
	{ { 2, 12 }, { 1, 13 }, 1 },
	{ { 1, 13 }, { 1, 6 }, 4 },
	{ { 5, 2 }, { 2, 9 }, 2 },
	{ { 2, 9 }, { 2, 12 }, 3 },
	{ { 2, 9 }, { 2, 7 }, 4 },
	{ { 2, 9 }, { 1, 10 }, 1 },
	{ { 1, 10 }, { 1, 8 }, 4 },
	{ { 1, 10 }, { 1, 13 }, 3 },
	{ { 6, 1 }, { 2, 14 }, 2 },
	{ { 2, 14 }, { 2, 9 }, 5 },
	{ { 2, 14 }, { 1, 15 }, 1 },
	{ { 1, 15 }, { 1, 10 }, 5 },
};

ideal_t reference_le[20][REFERENCE_POSET_N] = {
	{ 3, 1, 5, 6, 4, 2 },
	{ 3, 1, 6, 5, 4, 2 },
	{ 3, 5, 1, 6, 4, 2 },
	{ 3, 5, 6, 1, 4, 2 },
	{ 3, 5, 6, 4, 1, 2 },
	{ 3, 6, 1, 5, 4, 2 },
	{ 3, 6, 5, 1, 4, 2 },
	{ 3, 6, 5, 4, 1, 2 },
	{ 5, 3, 1, 6, 4, 2 },
	{ 5, 3, 6, 1, 4, 2 },
	{ 5, 3, 6, 4, 1, 2 },
	{ 5, 6, 3, 1, 4, 2 },
	{ 5, 6, 3, 4, 1, 2 },
	{ 5, 6, 4, 3, 1, 2 },
	{ 6, 3, 1, 5, 4, 2 },
	{ 6, 3, 5, 1, 4, 2 },
	{ 6, 3, 5, 4, 1, 2 },
	{ 6, 5, 3, 1, 4, 2 },
	{ 6, 5, 3, 4, 1, 2 },
	{ 6, 5, 4, 3, 1, 2 }
};

count_t reference_counts[16] = { 20, 20, 12, 3, 1, 1, 1, 2, 1, 6, 2, 3, 2, 1, 8, 2 };

/////////////////////// POSET 2 //////////////////////////////////
// From diagrams in "Efficient computation of rank probabilities in posets" by Karel De Loof 

ideal_t reference_poset2[4][2] = {
	{ 2, 4 },
	{ 1, 3 },
	{ 1, 4 },
	{ 3, 5 }
};

t_edge reference_tree2[10] = {
	{ { 0, 0 }, { 5, 1 }, 5 },
	{ { 5, 1 }, { 4, 2 }, 4 },
	{ { 4, 2 }, { 3, 3 }, 3 },
	{ { 3, 3 }, { 2, 4 }, 2 },
	{ { 2, 4 }, { 1, 5 }, 1 },
	{ { 3, 3 }, { 1, 6 }, 1 },
	{ { 4, 2 }, { 2, 7 }, 2 },
	{ { 5, 1 }, { 3, 8 }, 3 },
	{ { 0, 0 }, { 4, 9 }, 4 },
	{ { 4, 9 }, { 2, 10 }, 2 },
};

t_edge reference_lattice2[15] = {
	{ { 0, 0 }, { 5, 1 }, 5 },
	{ { 5, 1 }, { 4, 2 }, 4 },
	{ { 4, 2 }, { 3, 3 }, 3 },
	{ { 3, 3 }, { 2, 4 }, 2 },
	{ { 2, 4 }, { 1, 5 }, 1 },
	{ { 3, 3 }, { 1, 6 }, 1 },
	{ { 1, 6 }, { 1, 5 }, 2 },
	{ { 4, 2 }, { 2, 7 }, 2 },
	{ { 2, 7 }, { 2, 4 }, 3 },
	{ { 5, 1 }, { 3, 8 }, 3 },
	{ { 3, 8 }, { 3, 3 }, 4 },
	{ { 0, 0 }, { 4, 9 }, 4 },
	{ { 4, 9 }, { 4, 2 }, 5 },
	{ { 4, 9 }, { 2, 10 }, 2 },
	{ { 2, 10 }, { 2, 7 }, 5 },
};

ideal_t reference_le2[9][REFERENCE_POSET2_N] = {
	{ 2, 1, 4, 3, 5 },
	{ 2, 1, 3, 4, 5 },
	{ 2, 1, 3, 5, 4 },
	{ 1, 2, 4, 3, 5 },
	{ 1, 2, 3, 4, 5 },
	{ 1, 2, 3, 5, 4 },
	{ 1, 3, 2, 4, 5 },
	{ 1, 3, 2, 5, 4 },
	{ 1, 3, 5, 2, 4 }
};

count_t reference_counts2[11] = { 9, 5, 3, 2, 1, 1, 1, 1, 2, 4, 1 };
