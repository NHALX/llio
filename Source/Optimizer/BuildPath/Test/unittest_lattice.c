#ifdef __OSX__
#include <OpenCL/cl.h>
#else
#include <CL/cl.h>
#endif
#ifndef _MSC_VER
#include <alloca.h>
#else
#define alloca _alloca
#endif
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <stdio.h>

#include "../lattice.h"
#include "../tree.h"


#define PRINT
#define MATHEMATICA_OUTPUT

#include "reference_lattice.h"



/*
 * MATHEMATICA code to display the tree, and lattice in two different styles.
 * Note: you need to remove the last comma in each list output manually, heh.

  TreePlot[idealtree, Automatic, idealtree[[1]][[1]][[1]],VertexLabeling -> True]
  LayeredGraphPlot[ideallattice, VertexLabeling -> True]
  Graph[Sort[latticeEdges /. ({a_ -> b_, c_}) :> Labeled[Sort[b] \[DirectedEdge] Sort[a],c]], VertexLabels->"Name"]

 *
 */
//#undef MATHEMATICA_OUTPUT
#ifdef MATHEMATICA_OUTPUT
#define OUTPUT_FMT_EDGE			"{\"%d [%d]\" -> \"%d [%d]\", %d}, \n"
#define OUTPUT_FMT_EDGE_IS		"{%s -> %s, %d},\n"
#else
#define OUTPUT_FMT_EDGE			"{{%d,%d},{%d,%d}, %d}, \n"
#define OUTPUT_FMT_EDGE_IS		"{{%s,%s}, %d},\n"
#endif


static void *test_edge(struct ideal_lattice *il, struct vertex *v, struct vertex *v2, size_t child_n, void *unused)
{
	t_edge edge;
	t_ctx *ctx = (t_ctx*)unused;
	int same;
	memset(&edge, 0, sizeof edge);
	edge.a.label = v->label;
	edge.b.label = v2->label;
	edge.a.index = p_index(v, P_ALLOC_VERTEX);
	edge.b.index = p_index(v2, P_ALLOC_VERTEX);
	edge.label   = il->ideals[INDEX2(il->ctx.max_neighbors, edge.a.index, child_n)];

	assert(ctx->index >= 0 && ctx->index < ctx->length);
	same = memcmp(&edge, &ctx->reference[ctx->index], sizeof edge);
	assert(same == 0);
	
	ctx->index++;
	return (void*)ctx;
}

static void *print_edge(struct ideal_lattice *il, struct vertex *v, struct vertex *v2, size_t child_n, void *unused)
{
	size_t max_n = il->ctx.max_neighbors;
	size_t v1i, v2i;
	v1i = p_index(v, P_ALLOC_VERTEX);
	v2i = p_index(v2, P_ALLOC_VERTEX);
	c_ideal_t ideal = il->ideals[INDEX2(max_n, v1i, child_n)];

	printf(OUTPUT_FMT_EDGE, v->label, v1i, v2->label, v2i, ideal);
	return 0;
}


static void show_mask(struct ctx *x, c_mask_t mask, char *buf)
{
	char number[(sizeof(mask)* 8) + 1];
	int nonempty = 0;
	int j;

	buf[0] = '{';
	buf[1] = 0x0;
	buf[2] = 0x0;

	for (j = 0; j < (sizeof mask) * 8; j++)
	{
		if (1 << j & mask)
		{
			int n = (j + 1);
			//n = x->linear_extension[n - 1];

			//itoa(n, number, 10);
			sprintf(number, "%d", n);
			strcat(buf, number);
			strcat(buf, ",");
			nonempty = 1;
		}
	}
	if (nonempty)
		buf[strlen(buf) - 1] = '}';
	else
		buf[1] = '}';
}

static void *print_edgeIS(struct ideal_lattice *il, struct vertex *v, struct vertex *v2, size_t child_n, void *state)
{
	c_mask_t le = (c_mask_t) state;
	c_mask_t next;
	static char s1[(C_IDEAL_T_MAX * 2) + 3];
	static char s2[(C_IDEAL_T_MAX * 2) + 3];

	size_t v1i      = p_index(v, P_ALLOC_VERTEX);
	c_ideal_t ideal = il->ideals[INDEX2(il->ctx.max_neighbors, v1i, child_n)];

	show_mask(&il->ctx, le, s1);
	next = le & ~(1 << (ideal - 1));
	show_mask(&il->ctx, next, s2);

	printf(OUTPUT_FMT_EDGE_IS, s1, s2, ideal);
	return (void*)next;
}

static void tree_map(struct ideal_lattice *il, struct vertex *v, int include_impred, void* (*callback)(), void *ctx)
{
	c_iterator i;

	FOR_X_IN_CHILDREN(&il->ctx, i, v)
	{
		if (include_impred || !CHILD_IMPRED(v, i))
		{
			struct vertex *v2  = C_X(i);
			void *ctx2 = callback(il, v, v2, C_N(i), ctx);

			// By testing this here we avoid having to mark nodes as visited.
			// Following just the paths part of the original spanning tree (tree of ideals)
			// guarantees that nodes are visited only once.
			if (!CHILD_IMPRED(v, i)) 
				tree_map(il, v2, include_impred, callback, ctx2);
		}
	}

}



//////////////////////////////////////////////
void
print_extension(c_ideal_t *le, size_t n, void **unused)
{
	size_t i;

	printf("{");
	for (i = 0; i < n; ++i)
		printf("%d,", le[i]);
	printf("}\n");
}

void
copy_extension(c_ideal_t *le, size_t n, c_ideal_t **storage)
{
	memcpy(*storage, le, n);
	(*storage) += n;
}

typedef void(*linext_cb_t)(c_ideal_t *, size_t, void **);

void
all_extensions(struct ideal_lattice *il, c_index_t index, c_ideal_t *le, size_t le_n, linext_cb_t callback, void **ctx)
{
	size_t j,i = il->max_neighbors*index;

	c_index_t *neighbors = &il->neighbors[i];
	c_ideal_t *ideals    = &il->ideals[i];

	if (le_n == 0)
	{
		assert(neighbors[0] == INVALID_NEIGHBOR);
		callback(le, il->linext_width, ctx);
		return;
	}
	
	for (j = 0; j < il->max_neighbors; ++j)
	{
		if (neighbors[j] == INVALID_NEIGHBOR)
			continue;

		le[le_n - 1] = ideals[j];
		all_extensions(il, neighbors[j], le, le_n - 1, callback, ctx);
	}
	
}

size_t lexicographic_sort_len = 0;

int lexicographic_cmp(const void *arg1, const void *arg2)
{
	size_t i;
	c_ideal_t *a = ((c_ideal_t *)arg1);
	c_ideal_t *b = ((c_ideal_t *)arg2);

	for (i = 0; i < lexicographic_sort_len; ++i)
	{
		int result = (int)a[i] - (int)b[i];
		if (result != 0)
			return result;
	}

	return 0;
}

#define PRINT_STRUCT(NAME,F) do { \
	printf("%s = {\n", NAME); \
	F; \
	printf("};\n"); \
} while (0)

void
unittest_lattice_p(
	int quiet,
	c_ideal_t poset[][2], size_t poset_n,
	size_t    lattice_n, 
	t_edge    *r_tree,    size_t r_tree_n,
	t_edge    *r_lattice, size_t r_lattice_n,
	c_ideal_t *r_le,      size_t r_le_n,
	c_count_t *r_counts,  size_t r_counts_n
	)
{
	t_ctx test;
	size_t i;
	struct ideal_lattice lattice;

	c_mask_t le = C_MASK_MAX >> ((sizeof(le)* 8) - lattice_n);
	c_ideal_t *le_ptr,*le_storage;
	c_ideal_t *le_set = alloca(lattice_n*sizeof *le_set);
	size_t rle_size;



	assert(lattice_create(poset, poset_n, lattice_n, &lattice) == G_SUCCESS);

	if (!quiet)
	{

		PRINT_STRUCT("idealtree", tree_map(&lattice, lattice.ctx.root, 0, &print_edge, 0));
		PRINT_STRUCT("ideallattice", tree_map(&lattice, lattice.ctx.root, 1, &print_edge, 0));

		PRINT_STRUCT("idealCounts", for (i = 0; i < lattice.vertex_count; ++i)
			printf("%d,", lattice.counts[i]));
	}

	INIT_T_CTX(test, r_tree, r_tree_n);
	tree_map(&lattice, lattice.ctx.root, 0, &test_edge, &test);
	INIT_T_CTX(test, r_lattice, r_lattice_n);
	tree_map(&lattice, lattice.ctx.root, 1, &test_edge, &test);

	lattice_valmap(&lattice);

	if (!quiet)
	{
		PRINT_STRUCT("latticeEdges", tree_map(&lattice, lattice.ctx.root, 1, &print_edgeIS, (void*)le));
		all_extensions(&lattice, lattice.source, le_set, lattice.linext_width, (linext_cb_t) &print_extension, 0);
	}

	rle_size = r_le_n * sizeof *r_le * lattice_n;
	le_ptr = le_storage = malloc(rle_size); 
	all_extensions(&lattice, lattice.source, le_set, lattice.linext_width, (linext_cb_t) &copy_extension, (void**)&le_ptr);

	lexicographic_sort_len = lattice_n;
	qsort(r_le, r_le_n, lexicographic_sort_len, &lexicographic_cmp);
	qsort(le_storage, r_le_n, lexicographic_sort_len, &lexicographic_cmp);
	assert(memcmp(r_le, le_storage, rle_size) == 0);

	assert(memcmp(lattice.counts, r_counts, r_counts_n) == 0);

	lattice_free(&lattice);
}


#ifdef PRINT
#define SIMULATION_RUNS 1
#else
#define SIMULATION_RUNS 100000000
#endif

extern void unittest_u_list();
extern void unittest_tree();


void
unittest_lattice(int quiet)
{
	unittest_u_list();
	unittest_tree();

#define LEN(R) sizeof R / sizeof *R


	unittest_lattice_p(quiet,
		reference_poset2, sizeof reference_poset2 / sizeof *reference_poset2, 
		REFERENCE_POSET2_N,
		reference_tree2, LEN(reference_tree2),
		reference_lattice2, LEN(reference_lattice2),
		(c_ideal_t*) reference_le2, LEN(reference_le2),
		reference_counts2, LEN(reference_counts2)
		);
	
	unittest_lattice_p(quiet,
		reference_poset, sizeof reference_poset / sizeof *reference_poset,
		REFERENCE_POSET_N,
		reference_tree, LEN(reference_tree),
		reference_lattice, LEN(reference_lattice),
		(c_ideal_t*)reference_le, LEN(reference_le),
		reference_counts, LEN(reference_counts)
		);

#undef LEN
	printf("unittest_lattice: passed tests.\n");
	return;
}


