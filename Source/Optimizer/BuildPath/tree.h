#ifndef _TREE_H_
#define _TREE_H_

#include "x_types.h"
#include "u_list.h"

//typedef uint32_t t_mask_t;


typedef union 
{
	struct vertex *allbits;
	struct {
#ifdef LITTLE_ENDIAN
		#define VPTR_FLAGS_LEN 3

		unsigned int t_hidden : 1;
		unsigned int t_impred : 1;
		unsigned int t_pushed : 1;
		unsigned int t_ptr    : (sizeof(void*)*8) - VPTR_FLAGS_LEN; 

		#define C_X_VP(I) (*(vertex_ptr*)&(UL_X(I)))
		#define C_X(I) ((struct vertex *) (C_X_VP(I).tag.t_ptr << VPTR_FLAGS_LEN))
#else
		#error "TODO: BIG_ENDIAN support."
#endif

	} tag;

} vertex_ptr;

#define C_N(I) UL_N(I)

struct vertex
{
#ifndef NDEBUG
	size_t        index; // this can be optimized out since its easy to calculate the offset from the pool start
#endif
	struct vertex *parent;
	struct u_list *children;
	//struct u_list *impred;
	c_ideal_t      label;
	//t_mask_t       child_mask;
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

	struct vertex *root;
};



typedef struct u_iterator c_iterator;


#define CHILD_HIDDEN(PARENT,I)   C_X_VP(I).tag.t_hidden == 1 //((V)->child_mask & (1 << ((I).n))) // N must be the child's position in a regular traversal - not reverse
#define CHILD_IMPRED(PARENT,I)   C_X_VP(I).tag.t_impred == 1
#define CHILD_MARKED(PARENT,I)   C_X_VP(I).tag.t_pushed == 1
#define CHILD_HIDE(PARENT,I)     C_X_VP(I).tag.t_hidden = 1  //(V)->child_mask |= 1 << (I).n
#define CHILD_MARK(PARENT,I)     C_X_VP(I).tag.t_pushed = 1 


#define FOR_X_IN_CHILDREN(X,I,V)         FOR_X_IN_LIST(I,(V)->children)
#define FOR_X_IN_CHILDREN_REVERSE(X,I,V) FOR_X_IN_LIST_REVERSE(I,(V)->children)



int tree_addchild(struct ctx *x, struct vertex * p, struct vertex * i);
int tree_pushimpred(struct ctx *x, struct vertex * p, struct vertex * c);
//int tree_delchild(struct ctx *x, struct vertex * p, struct vertex * i);
int tree_hidechild(struct ctx *x, struct vertex * p, struct vertex * i);
struct vertex *tree_firstchild(struct ctx *x, struct vertex *v);
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

#endif
