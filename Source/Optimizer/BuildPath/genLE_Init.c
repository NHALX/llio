
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
glbinit_lattice()
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

STATIC int Toposort(unsigned char *graph, size_t dim, unsigned char *LE);


int CtxInit(struct ctx *ctx, c_ideal_t edges[][2], size_t nedges, size_t n)
{
	size_t i, j;
	unsigned char *graph;
	unsigned char *LE;
	void *adjacency;

	graph     = calloc(n, n);
	LE        = malloc(n);
	adjacency = calloc(n, n);

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

	for (i = 0; i < nedges; ++i)
	{
		size_t x = edges[i][0] - 1;
		size_t y = edges[i][1] - 1;
		assert(x <= n && y <= n);
		graph[INDEX2(n, x, y)] = 1;
	}
	

	if (Toposort(graph, n, LE) != G_SUCCESS)
		goto FAIL;

	for (i = 0; i < ctx->adjacency_dim; ++i)
		for (j = 0; j < ctx->adjacency_dim; ++j)
			IMMEDIATE_PRED(ctx, i, j) = graph[INDEX2(n, LE[i] - 1, LE[j] - 1)];

	free(graph);
	return G_SUCCESS;
}


void CtxFree(struct ctx *ctx)
{
	free(ctx->adjacency);
	free(ctx->linear_extension);
}



STATIC int
Toposort(unsigned char *graph, size_t dim, unsigned char *LE)
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

STATIC c_count_t AssignCount(const struct graph *graph, unsigned char *visited, c_index_t vertex_index)
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
			lef += AssignCount(graph, visited, next);
	}

	graph->counts[vertex_index] = lef;
	return lef;
}

// TODO: use lattice struct instead of graph
int lattice_count(struct graph *g)
{
	unsigned char *visited;
	GUARD(visited = calloc(1, g->vertex_count));
	
	AssignCount(g, visited, 1);
	g->counts[g->vertex_count-1] = 1;
	
	free(visited);
	return G_SUCCESS;
}

/////////////////////// Tree of ideals //////////////////////////////////

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
	GUARD(Right(x, i, r, root));
	r->parent = root;
	GUARD(tree_addchild(x, root, r));
	r->label = i;
	return root;
}


////////////////////// hasse diagram of the lattice of ideals //////////////////////


STATIC int Push(struct ctx *x, c_ideal_t *edges, size_t w, struct vertex *vertex, struct vertex *c, c_ideal_t ideal)
{
	int i;
	uintptr_t index = p_index(vertex, P_ALLOC_VERTEX);
	c_ideal_t *neighbors = &edges[INDEX2(w, index, 0)];

	assert(ideal != 0);
	assert(vertex->edge_len < w);

	GUARD(tree_pushimpred(x, vertex, c));

	for (i = vertex->edge_len; i > 0; --i)
		neighbors[i] = neighbors[i - 1];

	neighbors[0] = ideal;
	vertex->edge_len++;
	return G_SUCCESS;
}


// OPTIMIZATION: This loop is a major hotspot. Accounts for ~2/3 of total execution time.
STATIC int Process(struct ctx *x, c_ideal_t *edges, size_t edge_w, size_t k, struct vertex *v)
{
	c_iterator i;
	
	FOR_X_IN_CHILDREN(x, i, v->parent)
	{
		struct vertex *v2 = C_X(i);
		struct vertex *v3;
		c_ideal_t ideal;

		if (v2 == v)
			return G_SUCCESS;
		
		assert(v2->label > k && v->label == k);
		GUARD(v3 = tree_firstchild(x, v2));
		assert(v3->label == k);

		ideal = edges[INDEX2(edge_w, p_index(v->parent, P_ALLOC_VERTEX), C_N(i))];
		GUARD(Push(x, edges, edge_w, v, v3, ideal));
	}

	return G_SUCCESS;
}


STATIC int GroupAll(struct u_list **E)
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

int
RecordEdges(struct ctx *x, c_ideal_t *edges, size_t edge_w, struct vertex *v)
{
	c_iterator i;
	size_t index = p_index(v, P_ALLOC_VERTEX);

	FOR_X_IN_CHILDREN(x, i, v)
	{
		c_ideal_t ideal = C_X(i)->label;
		edges[INDEX2(edge_w, index, v->edge_len)] = ideal;
		v->edge_len++;
		RecordEdges(x, edges, edge_w, C_X(i));
	}

	return G_SUCCESS;
}


STATIC int BuildLattice(struct ctx *x, c_ideal_t *edges, size_t edge_w, struct vertex *root, unsigned char n)
{
	int k;
	struct u_iterator i;
	struct u_list **E = alloca(n * sizeof(*E)); // limits to UCHAR_MAX
	
	memset(E, 0, n * sizeof *E);

	GUARD(RecordEdges(x, edges, edge_w, root));
	GUARD(GroupAll(E));

	for (k = n; k >= 1; k--)
	{
		FOR_X_IN_LIST_REVERSE(i, E[k-1])
			GUARD(Process(x, edges, edge_w, k, UL_X(i)));

		FOR_X_IN_LIST_REVERSE(i, E[k-1])
		{
			struct vertex *v = UL_X(i);
			tree_hidechild(x, v->parent, v);
		}
	}

	return G_SUCCESS;
}




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

int lattice_create(c_ideal_t p_relations[][2], size_t p_reln, size_t n, struct ideal_lattice *lattice)
{
	c_index_t *neighbors;
	c_ideal_t *edges;
	size_t i, slen;
	struct p_iterator p;

	GUARD(CtxInit(&lattice->ctx, p_relations, p_reln, n));
	GUARD(lattice->ctx.root = Left(&lattice->ctx, n)); // sets vertex_count

	slen      = lattice->ctx.vertex_count * lattice->ctx.max_neighbors;
	edges     = malloc(slen * sizeof *edges);
	neighbors = malloc(slen * sizeof *neighbors);

	if (!edges || !neighbors)
	{
	FAIL:
		G_FREE(edges);
		G_FREE(neighbors);
		CtxFree(&lattice->ctx);
		return G_ERROR;
	}

	//print_memusage(slen * sizeof*edges, slen * sizeof *neighbors);

	if (BuildLattice(&lattice->ctx, edges, lattice->ctx.max_neighbors, lattice->ctx.root, n) != G_SUCCESS)
		goto FAIL;

	//print_memusage(slen * sizeof*edges, slen * sizeof *neighbors);

	FOR_X_IN_POOLS(p, P_ALLOC_VERTEX)
	{
		struct vertex *v = P_X(p);
		c_iterator j;
		size_t k;

		i = p.global_index * lattice->ctx.max_neighbors;

		FOR_X_IN_CHILDREN(x, j, v)
			neighbors[i + C_N(j)] = p_index(C_X(j), P_ALLOC_VERTEX);

		for (k = C_N(j); k < lattice->ctx.max_neighbors; ++k)
			neighbors[i + k] = 0;
	}

	lattice->ideals           = edges;
	lattice->neighbors        = neighbors;
	lattice->source           = p_index(lattice->ctx.root, P_ALLOC_VERTEX);
	lattice->sink             = n;
	lattice->vertex_count     = lattice->ctx.vertex_count;
	lattice->max_neighbors    = lattice->ctx.max_neighbors;
	lattice->extension_length = n;

	for (i = 0; i < lattice->ctx.max_neighbors; ++i)
		assert(neighbors[INDEX2(lattice->ctx.max_neighbors, lattice->sink, i)] == 0);

	return G_SUCCESS;
}

// Map values from their internal representation to the given initial linear extension.
// In other words: for i in {1,2..n}. i -> lattice.linear_extension[i]
void lattice_valmap(struct ideal_lattice *lattice)
{
	size_t i, slen = lattice->ctx.vertex_count * lattice->ctx.max_neighbors;

	for (i = 0; i < slen; ++i)
		lattice->ideals[i] = lattice->ctx.linear_extension[lattice->ideals[i] - 1];
}


void lattice_free(struct ideal_lattice *lattice)
{
	CtxFree(&lattice->ctx);
	p_release(P_ALLOC_ULIST);
	p_release(P_ALLOC_VERTEX);
}