
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <stdio.h>
#ifndef _MSC_VER
#include <alloca.h>
#else
#define alloca _alloca
#endif
#include "x_types.h"
#include "genLE_Init.h"

#include "tree.h"
#include "u_list.h"

////////////////////// GLOBAL INIT //////////////////////

void
__glbinit__lattice()
{
	size_t type_size[2];

	type_size[P_ALLOC_ULIST]  = sizeof (struct u_list);
	type_size[P_ALLOC_VERTEX] = sizeof (struct vertex);

	if (p_init(type_size, 2) != G_SUCCESS)
	{
		printf("[INIT]: Failure at. %s:%d\n", __FILE__, __LINE__);
		abort();
	}
}

////////////////////// INIT //////////////////////

STATIC int toposort(unsigned char *graph, size_t dim, unsigned char *LE);


int ctx_init(struct ctx *ctx, c_ideal_t edges[][2], size_t nedges, size_t n)
{
	size_t i, j;
	unsigned char *graph = calloc(n, n);
	unsigned char *LE    = malloc(n);
	void *adjacency      = calloc(n, n);

	if (!graph || !LE || !adjacency)
	{
	FAIL:
		G_FREE(graph);
		G_FREE(LE);
		G_FREE(adjacency);
		return G_ERROR;
 	}

	memset(ctx, 0, sizeof *ctx);
	ctx->adjacency        = adjacency; // filled by IMMEDIATE_PRED reference below
	ctx->adjacency_dim    = n;
	ctx->linear_extension = LE;

	// TODO: uncomment this
	//for (i = 0; i < nedges; ++i)
	//	graph[INDEX2(n, edges[i][0] - 1, edges[i][1] - 1)] = 1;

	if (toposort(graph, n, LE) != G_SUCCESS)
		goto FAIL;

	for (i = 0; i < ctx->adjacency_dim; ++i)
		for (j = 0; j < ctx->adjacency_dim; ++j)
			IMMEDIATE_PRED(ctx, i, j) = graph[INDEX2(n, LE[i] - 1, LE[j] - 1)];

	free(graph);
	return G_SUCCESS;
}


void ctx_free(struct ctx *ctx)
{
	free(ctx->adjacency);
	free(ctx->linear_extension);
}



STATIC int
toposort(unsigned char *graph, size_t dim, unsigned char *LE)
{
	unsigned char *g;
	size_t le_i = 0;
	size_t i, j;

	GUARD(g = malloc(dim*dim));
	memcpy(g, graph, dim*dim);


	while (le_i < dim)
	{
		for (j = 0; j < dim; ++j)
		{
			int in_degree = 0;
			int deleted = 0;

			for (i = 0; i < dim && !in_degree; ++i)
			{
				switch (g[INDEX2(dim, i, j)]){
				case 0xff: deleted += 1; break;
				case 0x01: in_degree += 1; break;
				}
			}

			if (!in_degree && deleted != dim)
			{
				// mark as deleted
				for (i = 0; i < dim; ++i)
				{
					g[INDEX2(dim, i, j)] = 0xff;
					g[INDEX2(dim, j, i)] = 0xff;
				}
				LE[le_i++] = j + 1;
				break;
			}
		}
	}

	free(g);
	return G_SUCCESS;
}



////////////////////// count data //////////////////////

#include "le.h"

#define GET_NEIGHBORS(P,S,I) (P + (I*S))

STATIC c_count_t assign_count(const struct graph *graph, unsigned char *visited, c_index_t vertex_index)
{
	unsigned int i;
	c_count_t lef = 0;
	c_index_t *neighbors = GET_NEIGHBORS(graph->adjacency, graph->max_neighbors, vertex_index);
	c_index_t next;

	visited[vertex_index] = 1;

	for (i = 0; i < graph->max_neighbors; ++i)
	{
		if (neighbors[i] == 0)
			continue;

		next = neighbors[i];
		
		if (next == graph->end)
			lef = 1;

		else if (visited[next])
			lef += graph->counts[next];

		else
			lef += assign_count(graph, visited, next);
	}

	graph->counts[vertex_index] = lef;
	return lef;
}

int init_count(struct graph *g)
{
	unsigned char *visited;
	GUARD(visited = calloc(1, g->vertex_count));
	
	assign_count(g, visited, 1);
	g->counts[g->vertex_count-1] = 1;
	
	free(visited);
	return G_SUCCESS;
}


/////////////////////// lattice supporting functions ////////////////////

STATIC int push(struct ctx *x, c_ideal_t *edges, size_t w, struct vertex *vertex, struct vertex *c, c_ideal_t ideal)
{
	assert(ideal != 0);
	GUARD(ul_push(&vertex->impred, c));
	uintptr_t index = p_index(vertex, P_ALLOC_VERTEX);
	edges[INDEX2(w, index, vertex->edge_len)] = ideal;
	vertex->edge_len++;
	return G_SUCCESS;
}

STATIC int push_children(struct ctx *x, c_ideal_t *edges, size_t w, struct vertex *vertex)
{
	c_iterator i;

	FOR_X_IN_CHILDREN(x, i, vertex)
		GUARD(push(x, edges, w, vertex, C_X(i), C_X(i)->label));

	return G_SUCCESS;
}

STATIC int Right(struct ctx *x, int index, struct vertex *r, struct vertex *root)
{	
	if (r->children_len)
	{
		c_iterator j;

		FOR_X_IN_CHILDREN_REVERSE(x, j, r)
		{
			struct vertex *s = C_X(j);
			struct vertex *t;
			
			if (IMMEDIATE_PRED(x, s->label - 1, index - 1))
				continue;

			GUARD(t = tree_vertex(x));
			t->parent = root;
			GUARD(tree_addchild(x, root, t));
			t->label = s->label;
			GUARD(Right(x, index, s, t));
		}
	}

	return G_SUCCESS;
}


STATIC struct vertex * Left(struct ctx *x, int i)
{
	struct vertex *root;
	struct vertex *r;

	GUARD(root = tree_vertex(x));
	
	if (i == 0)
		return root;
	
	GUARD(r = Left(x, i - 1));
	GUARD(Right(x,i,r,root));
	r->parent = root;
	GUARD(tree_addchild(x, root, r));
	r->label = i;
	return root;
}



////////////////////// hasse diagram of the lattice of ideals //////////////////////


STATIC int process(struct ctx *x, c_ideal_t *edges, size_t edge_w, size_t k, struct vertex *v)
{
	struct u_iterator j;
	struct vertex *v2, *v3;

	// OPTIMIZATION: This loop is a major hotspot. Accounts for ~2/3 of total execution time.
	FOR_X_IN_LIST_REVERSE(j, &v->parent->impred)
	{
		c_ideal_t ideal;
		v2 = UL_X(j);

		if (v2 == v)
			goto OUT;
		
		assert(v2->label > k && v->label == k);
		v3 = ul_first(&v2->children);
		assert(v3->label == k);
		
		ideal = edges[INDEX2(edge_w, p_index(v->parent, P_ALLOC_VERTEX), UL_N(j))];
		GUARD(push(x, edges, edge_w, v, v3, ideal));
	}
OUT:

	GUARD(push_children(x, edges, edge_w, v));
	return G_SUCCESS;
}


STATIC int groupAll(struct u_lhead *E)
{
	struct p_iterator p;
 
	FOR_X_IN_POOLS(p, P_ALLOC_VERTEX)
	{
		struct vertex *v = P_X(p);
		c_ideal_t label = v->label;

		if (label != 0)
			GUARD(ul_push(&E[label - 1], v));
	}

	return G_SUCCESS;
}



STATIC int buildLattice(struct ctx *x, c_ideal_t *edges, size_t edge_w, struct vertex *root, unsigned char n)
{
	int k;
	struct u_iterator i;
	struct u_lhead *E = alloca(n * sizeof(*E)); // limits to UCHAR_MAX
	
	memset(E, 0, n * sizeof *E);

	GUARD(push_children(x, edges, edge_w, root));
	GUARD(groupAll(E));

	for (k = n; k >= 1; k--)
	{
		FOR_X_IN_LIST_REVERSE(i, &E[k-1])
			GUARD(process(x, edges, edge_w, k, UL_X(i)));

		FOR_X_IN_LIST_REVERSE(i, &E[k-1])
		{
			struct vertex *v = UL_X(i);
			tree_delchild(x, v->parent, v);
		}
	}

	return G_SUCCESS;
}




///// top level wrappers /////
void
print_memusage(size_t edges, size_t neighbors)
{
#define MB(X) ((float)X/1024/1024)

	printf("P_ALLOC_VERTEX:%f,P_ALLOC_ULIST:%f,edges=%f,neighbors=%f\n", 
		MB(p_memusage(P_ALLOC_VERTEX)),
		MB(p_memusage(P_ALLOC_ULIST)),
		//MB(p_memusage(P_ALLOC_COO)),
		MB(edges),
		MB(neighbors)
		);

#undef MB
}

int idealLattice(c_ideal_t p_relations[][2], size_t p_reln, size_t n, struct ideal_lattice *lattice)
{
	struct ctx x;
	struct vertex *root;
	c_index_t *neighbors;
	c_ideal_t *edges;
	size_t i, slen, result;
	struct p_iterator p;

	GUARD(ctx_init(&x, p_relations, p_reln, n));
	GUARD(root = Left(&x, n));

	slen      = x.vertex_count * x.max_neighbors;
	edges     = malloc(slen * sizeof *edges);
	neighbors = malloc(slen * sizeof *neighbors);

	if (!edges || !neighbors)
	{
	FAIL:
		G_FREE(edges);
		G_FREE(neighbors);
		ctx_free(&x);
		return G_ERROR;
	}

	print_memusage(slen * sizeof*edges, slen * sizeof *neighbors);

	if (buildLattice(&x, edges, x.max_neighbors, root, n) != G_SUCCESS)
		goto FAIL;

	print_memusage(slen * sizeof*edges, slen * sizeof *neighbors);

	FOR_X_IN_POOLS(p, P_ALLOC_VERTEX)
	{
		struct vertex *v = P_X(p);
		struct u_iterator j;
		size_t k;

		i = p.global_index * x.max_neighbors;

		FOR_X_IN_LIST(j, &v->impred)
			neighbors[i + UL_N(j)] = p_index(UL_X(j), P_ALLOC_VERTEX);

		for (k = UL_N(j); k < x.max_neighbors; ++k)
			neighbors[i + k] = 0;
	}

	for (i = 0; i < slen; ++i)
		edges[i] = x.linear_extension[edges[i] - 1];
	
	lattice->ideals           = edges;
	lattice->neighbors        = neighbors;
	lattice->source           = p_index(root, P_ALLOC_VERTEX);
	lattice->sink             = n;
	lattice->vertex_count     = x.vertex_count;
	lattice->max_neighbors    = x.max_neighbors;
	lattice->extension_length = n;

	for (i = 0; i < x.max_neighbors; ++i)
		assert(neighbors[INDEX2(x.max_neighbors, lattice->sink, i)] == 0);

	ctx_free(&x);
	return G_SUCCESS;
}

