#include <stdint.h>
#include <string.h>
#include "p_alloc.h"
#include "u_list.h"
#include "tree.h"

////////////////////// tree of ideals vertices //////////////////////

struct vertex *tree_vertex(struct ctx *x)
{
	struct vertex *v;
	GUARD(v = p_alloc(P_ALLOC_VERTEX));
	memset(v, 0, sizeof *v);

#ifdef DEBUG_VERTEX_INDEX
	v->index = x->vertex_count;
#endif
	x->vertex_count++;
	return v;
}

#define CHILD_SET(x, p, i) GUARD(ul_push(&p->children, i));

// This is assumed to only be called while creating the lattice from the tree.
int tree_pushimpred(struct ctx *x, struct vertex * p, struct vertex * c)
{
	vertex_ptr ptr;
	ptr.allbits      = c;
	ptr.tag.t_impred = 1;

	GUARD(ul_push(&p->children, ptr.allbits));
	return G_SUCCESS;
}

// This is assumed to only be called while building the first (spanning) tree of ideals.
int tree_addchild(struct ctx *x, struct vertex * p, struct vertex * i)
{
	vertex_ptr ptr;
	ptr.allbits = p;
	assert(ptr.tag.t_hidden == 0 && ptr.tag.t_impred == 0);

	CHILD_SET(x, ptr.allbits, i);
	p->children_len++;

	if (x->max_neighbors < p->children_len)
		x->max_neighbors = p->children_len;

	return G_SUCCESS;
}

int tree_hidechild(struct ctx *x, struct vertex *p, struct vertex *c)
{
	c_iterator i;
	vertex_ptr ptr;
	
	ptr.allbits = c;

	FOR_X_IN_CHILDREN(x,i,p)
	{
		if (C_X_VP(i).tag.t_ptr == ptr.tag.t_ptr)
		{
			CHILD_HIDE(p, i);
			
			p->children_len--;
			return G_SUCCESS;
		}
	}
	
	return G_ERROR;
}

struct vertex *tree_firstchild(struct ctx *x, struct vertex *v)
{
	c_iterator i;
	
	FOR_X_IN_CHILDREN(x, i, v)
	{
		if (!CHILD_HIDDEN(v, i) && !CHILD_IMPRED(v,i))
			return C_X(i);
	}
	return 0;
}
