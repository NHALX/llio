#define STATIC

#include "tree.h"
////////////////////// Vertex //////////////////////
typedef cl_uint c_mask_t;
#define C_MASK_MAX CL_UINT_MAX

struct ideal_lattice
{
	c_ideal_t *ideals;
	c_index_t *neighbors;
	c_index_t source;
	c_index_t sink;

	size_t vertex_count;
	size_t max_neighbors;
	size_t extension_length;

	struct ctx ctx;
};



void lattice_free(struct ideal_lattice *lattice);
int idealLattice(c_ideal_t p_relations[][2], size_t p_reln, size_t n, struct ideal_lattice *lattice);
