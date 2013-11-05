#define STATIC

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



STATIC __inline struct vertex *vertex(struct ctx *x)
{
	size_t i = x->vertex_count++;

	struct vertex *v = p_alloc(P_ALLOC_VERTEX);
	memset(v, 0, sizeof *v);
	v->index = i;
	return v;
}

/////////////////////// tree of ideals support functions ////////////////////

STATIC __inline int addChild(struct ctx *x, struct vertex * p, struct vertex * i)
{
	assert(p->children_len < UCHAR_MAX);

	GUARD(ul_push(&p->children, i));
	p->children_len++;

	if (x->max_neighbors < p->children_len)
		x->max_neighbors = p->children_len;

	return G_SUCCESS;
}

STATIC __inline void delChild(struct vertex *p, struct vertex *c)
{
	ul_unlink(&p->children, c);
	p->children_len--;
}

/////////////////////// lattice supporting functions ////////////////////

STATIC __inline void push_edge(c_ideal_t *edges, size_t w, struct vertex *vertex, c_ideal_t ideal)
{
	assert(ideal != 0);

	edges[INDEX2(w, vertex->index, vertex->edge_len)] = ideal;
	vertex->edge_len++;
}

STATIC __inline int push_children(c_ideal_t *edges, size_t w, struct vertex *vertex)
{
	struct u_iterator i;

	FOR_X_IN_LIST(i, &vertex->children)
	{
		struct vertex *c = UL_X(i);
		GUARD(ul_push(&vertex->impred, c));
		push_edge(edges, w, vertex, c->label);
	}

	return G_SUCCESS;
}
