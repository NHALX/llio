#define STATIC

#include "tree.h"
////////////////////// Vertex //////////////////////
typedef cl_uint c_mask_t;
#define C_MASK_MAX CL_UINT_MAX
#define INVALID_NEIGHBOR C_INDEX_T_MAX

struct ideal_lattice
{
	c_ideal_t *ideals;
	c_index_t *neighbors;
	c_count_t *counts;
	c_index_t source;
	c_index_t sink;

	size_t vertex_count;
	c_ideal_t max_neighbors;
	c_ideal_t linext_width;
	c_count_t linext_count;    // number of linear extensions
	struct ctx ctx;
};



void lattice_free(struct ideal_lattice *lattice);
int lattice_create(c_ideal_t p_relations[][2], size_t p_reln, size_t n, struct ideal_lattice *lattice);
void lattice_valmap(struct ideal_lattice *lattice);
void glbinit_lattice();
