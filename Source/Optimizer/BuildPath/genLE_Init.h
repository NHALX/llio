#define STATIC


////////////////////// Vertex //////////////////////
typedef cl_uint c_mask_t;

struct ideal_lattice
{
	c_ideal_t *ideals;
	c_index_t *neighbors;
	c_index_t source;
	c_index_t sink;

	size_t vertex_count;
	size_t max_neighbors;
	size_t extension_length;
};



