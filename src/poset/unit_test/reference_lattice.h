#include "types.h"


typedef struct {
	ideal_t label;
	size_t index;
} t_node;

typedef struct {
	t_node a;
	t_node b;
	ideal_t label;
} t_edge;

typedef struct {
	t_edge *reference;
	size_t length;
	size_t index;
} t_ctx;

#define INIT_T_CTX(T,R,LEN) do { \
	T.index = 0; \
	T.length = LEN; \
	T.reference = R; \
} while (0)


///////////////////////// REFERENCE POSET 1: youmuu's ghostblade //////////////////
#define REFERENCE_POSET_N 6

extern ideal_t reference_poset[5][2];
extern t_edge reference_tree[15];
extern t_edge reference_lattice[26];
extern ideal_t reference_le[20][REFERENCE_POSET_N];
extern count_t reference_counts[16];

/////////////////////// POSET 2 //////////////////////////////////
// From diagrams in "Efficient computation of rank probabilities in posets" by Karel De Loof 

#define REFERENCE_POSET2_N 5

ideal_t reference_poset2[4][2];
t_edge reference_tree2[10];
t_edge reference_lattice2[15];
ideal_t reference_le2[9][REFERENCE_POSET2_N];
count_t reference_counts2[11];
