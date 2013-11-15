#include <stdint.h>
#include <string.h>
#include "p_alloc.h"
#include "u_list.h"
#include "tree.h"
//#include "genLE_Init.h"
//#include <Judy.h>

////////////////////// tree of ideals vertices //////////////////////


struct vertex *tree_vertex(struct ctx *x)
{
	struct vertex *v;
	GUARD(v = p_alloc(P_ALLOC_VERTEX));
	memset(v, 0, sizeof *v);
#ifndef NDEBUG
	v->index = x->vertex_count;
#endif
	x->vertex_count++;
	return v;
}

#define CHILD_SET(x, p, i) GUARD(ul_push(&p->children, i));
//#define CHILD_UNSET(x, p, c) ul_unlink(&p->children, c);

/*
#define CHILD_SET(X,I,J) do { \
	Word_t * PValue = 0; \
	JLI(PValue, X->children, INDEX2(X->vertex_count_max, I->index, J->index)); \
if (PValue == PJERR) \
	return G_ERROR; \
	*PValue = J; \
} while (0)

#define CHILD_UNSET(X,I,J) do { \
	int      Rc_int; \
	JLD(Rc_int, X->children, INDEX2(X->vertex_count_max, I->index, J->index)); \
if (Rc_int != 1) \
	return G_ERROR; \
} while (0)
*/

int tree_pushimpred(struct ctx *x, struct vertex * p, struct vertex * c)
{
	vertex_ptr ptr;
	ptr.allbits      = c;
	ptr.tag.t_impred = 1;
	ptr.tag.t_pushed = 1;
	//GUARD(ul_append(&p->children, ptr.allbits));
	GUARD(ul_push(&p->children, ptr.allbits));
	return G_SUCCESS;
}

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
	//struct vertex *v2 = ul_first(&v->children);
	c_iterator i;
	
	FOR_X_IN_CHILDREN(x, i, v)
	{
		if (!CHILD_HIDDEN(v, i) && !CHILD_IMPRED(v,i))
			return C_X(i);
	}
	return 0;
}

/*
int tree_delchild(struct ctx *x, struct vertex *p, struct vertex *c)
{
	CHILD_UNSET(x, p, c);
	p->children_len--;
	return G_SUCCESS;
}
*/
/*
/////////////////////// tree of ideals support functions ////////////////////

STATIC int addChild(struct ctx *x, struct vertex * p, struct vertex * i)
{
	GUARD(ul_push(&p->children, i));
	p->children_len++;

	if (x->max_neighbors < p->children_len)
		x->max_neighbors = p->children_len;

	return G_SUCCESS;
}

STATIC void delChild(struct vertex *p, struct vertex *c)
{
	ul_unlink(&p->children, c);
	p->children_len--;
}
*/
/*
int compare(const void *arg1, const void *arg2)
{
	return (*(struct vertex**)arg2)->label - (*(struct vertex**)arg1)->label;
}

size_t children_len(struct ctx *x, struct vertex *v)
{
	Word_t   End;
	Word_t   Index;
	Word_t   n;

	Index = INDEX2(x->vertex_count_max, v->index, 0);
	End = INDEX2(x->vertex_count_max, v->index, x->vertex_count_max);
	JLC(n, x->children, Index, End);
	return n;
}

size_t children(struct ctx *x, struct vertex *v, struct vertex *output[])
{
	Word_t * PValue;
	Word_t   Index;                     // array index
	Word_t   n;
	size_t i;

	Index = INDEX2(x->vertex_count_max, v->index, 0);
	n = children_len(x, v);
	assert(n > 0);

	i = 0;
	JLF(PValue, x->children, Index);
	while (PValue && i < n)
	{
		//size_t j = (Index % x->vertex_count_max);
		output[i++] = (struct vertex *)*PValue;//p_ptr(j, P_ALLOC_VERTEX);
		JLN(PValue, x->children, Index);
	}

	qsort(output, n, sizeof *output, &compare);
	return n;
}

struct vertex *CHILD_FIRST(struct ctx *x, struct vertex *v)
{
	struct vertex **cs = alloca(sizeof *cs * children_len(x, v));
	children(x, v, cs);
	return cs[0];
}

*/
