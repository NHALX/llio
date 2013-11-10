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
	//v->index = x->vertex_count;
	x->vertex_count++;
	return v;
}

#define CHILD_SET(x, p, i) GUARD(ul_push(&p->children, i));
#define CHILD_UNSET(x, p, c) ul_unlink(&p->children, c);

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

int tree_addchild(struct ctx *x, struct vertex * p, struct vertex * i)
{
	CHILD_SET(x, p, i);
	p->children_len++;

	if (x->max_neighbors < p->children_len)
		x->max_neighbors = p->children_len;

	return G_SUCCESS;
}


int tree_delchild(struct ctx *x, struct vertex *p, struct vertex *c)
{
	CHILD_UNSET(x, p, c);
	p->children_len--;
	return G_SUCCESS;
}

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
