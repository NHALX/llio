#include <stdint.h>
#include <malloc.h>
#include <math.h>

#include "../p_alloc.h"
#include "../u_list.h"
#include "../tree.h"



void
unittest_tree()
{
#define UNITTEST_CHILD_N 2
	c_iterator iter;
	size_t i = 0;
	struct ctx x;
	struct vertex **vs = 0;
	void *test = 0;
//	size_t * PValue;
	size_t vertex_count_max;
	int result;

	x.adjacency_dim = UNITTEST_CHILD_N;
	x.max_neighbors = UNITTEST_CHILD_N;
	vertex_count_max = (size_t)pow(2, UNITTEST_CHILD_N);

	// children = 0;
	vs = calloc(vertex_count_max, sizeof*vs);
	assert(vs);
	for (i = 0; i < vertex_count_max; ++i)
	{
		vs[i] = tree_vertex(&x);
		assert(vs[i]);
		vs[i]->label = i;
		//vs[i]->index = i;
	}

	result = tree_addchild(&x, vs[0], vs[3]); assert(result == G_SUCCESS);
	result = tree_addchild(&x, vs[0], vs[2]); assert(result == G_SUCCESS);
	result = tree_addchild(&x, vs[0], vs[1]); assert(result == G_SUCCESS);

	i = 3;
	FOR_X_IN_CHILDREN_REVERSE(&x, iter, vs[0])
	{
		struct vertex *child = C_X(iter);
		assert(child->label == i);
		--i;
	}

	free(vs);
	p_release(P_ALLOC_VERTEX);
	p_release(P_ALLOC_ULIST);
}
