#include "x_types.h"
#include "u_list.h"

struct vertex
{
	//size_t        index; // this can be optimized out since its easy to calculate the offset from the pool start
	struct vertex  *parent;
	struct u_lhead children;
	struct u_lhead impred;
	c_ideal_t      label;

	// HARD_LIMITS: these will be too small if we have 2^256 vertices in our lattice...
	unsigned char children_len;
	unsigned char edge_len;
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




#define FOR_X_IN_CHILDREN(X,I,V)         FOR_X_IN_LIST(I,&(V)->children)
#define FOR_X_IN_CHILDREN_REVERSE(X,I,V) FOR_X_IN_LIST_REVERSE(I,&(V)->children)

#define C_X(I) ((struct vertex *)(UL_X(I)))
typedef struct u_iterator c_iterator;

int tree_addchild(struct ctx *x, struct vertex * p, struct vertex * i);
int tree_delchild(struct ctx *x, struct vertex * p, struct vertex * i);
struct vertex *tree_vertex(struct ctx *x);

/*
typedef struct  {
	size_t n;
	size_t i;
	struct vertex **cs;
} c_iterator;

size_t children_len(struct ctx *x, struct vertex *v);
size_t children(struct ctx *x, struct vertex *v, struct vertex *output[]);

#define FOR_X_IN_CHILDREN(I,X,V) \
for ((I).cs = alloca(sizeof *(I).cs * children_len(X, V)), \
	(I).i = 0, \
	(I).n = children((X), (V), (I).cs); (I).i < (I).n; ++((I).i))

#define C_X(I) ((I).cs[(I).i])

*/