#define STATIC

#include <limits.h>
#include "p_alloc.h"
#include "u_list.h"

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



struct vertex
{
	size_t            index; // this can be optimized out since its easy to calculate the offset from the pool start
	struct vertex    *parent;
	struct u_lhead children;
	struct u_lhead impred;
	c_ideal_t         label;

	// HARD_LIMITS: these will be too small if we have 2^256 vertices in our lattice...
	unsigned char     children_len; 
	unsigned char     edge_len;
};


#define INDEX2(N,I,J)			(((I)*(N))+(J))
#define IMMEDIATE_PRED(X,I,J)	((X)->adjacency[INDEX2((X)->adjacency_dim,I,J)])


struct ctx {
	size_t         max_neighbors;
	size_t         vertex_count;

	unsigned char *linear_extension;
	unsigned char *adjacency;
	size_t         adjacency_dim;
};



