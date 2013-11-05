#ifdef __OSX__
#include <OpenCL/cl.h>
#else
#include <CL/cl.h>
#endif

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include "le.h"



#include "genLE_Init.h"


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

STATIC void toposort(unsigned char *graph, size_t dim, unsigned char *LE);


void ctx_init(struct ctx *ctx, c_ideal_t edges[][2], size_t nedges, size_t n)
{
	size_t i, j;
	unsigned char *graph = _alloca(n*n);
	unsigned char *LE    = malloc(n);


	memset(ctx, 0, sizeof *ctx);
	memset(graph, 0, n*n);

	for (i = 0; i < nedges; ++i)
		graph[INDEX2(n, edges[i][0] - 1, edges[i][1] - 1)] = 1;

	toposort(graph, n, LE);

	ctx->adjacency = calloc(n*n, 1);
	ctx->adjacency_dim = n;
	ctx->linear_extension = LE;

	for (i = 0; i < ctx->adjacency_dim; ++i)
		for (j = 0; j < ctx->adjacency_dim; ++j)
			IMMEDIATE_PRED(ctx, i, j) = graph[INDEX2(n, LE[i] - 1, LE[j] - 1)];
}


void ctx_free(struct ctx *ctx)
{
	free(ctx->adjacency);
	free(ctx->linear_extension);
}



STATIC void
toposort(unsigned char *graph, size_t dim, unsigned char *LE)
{
	unsigned char *g = _alloca(dim*dim);
	size_t le_i = 0;
	size_t i, j;

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
}



////////////////////// count data //////////////////////



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

void init_count(struct graph *g)
{
	
	unsigned char *visited;
	visited = calloc(1, g->vertex_count);
	
	assign_count(g, visited, 1);
	g->counts[g->vertex_count-1] = 1;
	
	free(visited);
}

////////////////////// tree of ideals //////////////////////


STATIC int Right(struct ctx *x, int index, struct vertex *r, struct vertex *root)
{	
	if (r->children_len)
	{
		struct u_iterator j;

		FOR_X_IN_LIST_REVERSE(j, &r->children)
		{
			struct vertex *s = UL_X(j);
			struct vertex *t;
			
			if (IMMEDIATE_PRED(x, s->label - 1, index - 1))
				continue;

			t = vertex(x);
			t->parent = root;
			GUARD(addChild(x, root, t));
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

	GUARD(root = vertex(x));
	
	if (i == 0)
		return root;
	
	GUARD(r = Left(x, i - 1));
	GUARD(Right(x,i,r,root));
	r->parent = root;
	GUARD(addChild(x, root, r));
	r->label = i;
	return root;
}



////////////////////// hasse diagram of the lattice of ideals //////////////////////


STATIC __inline int process(c_ideal_t *edges, size_t edge_w, size_t k, struct vertex *v)
{
	struct u_iterator j;
	struct vertex *v2, *v3;

	// OPTIMIZATION: This loop is a major hotspot. Accounts for ~2/3 of total execution time.
	FOR_X_IN_LIST_REVERSE(j, &v->parent->impred)
	{
		v2 = UL_X(j);

		if (v2 == v)
			break;
		
		v3 = ul_first(&v2->children);
		
		assert(v2->label > k && v->label == k);
		assert(v3->label == k);
		
		GUARD(ul_push(&v->impred, v3));
		push_edge(edges, edge_w, v, edges[INDEX2(edge_w, v->parent->index, UL_N(j))]);
	}

	GUARD(push_children(edges, edge_w, v));
	return G_SUCCESS;
}


STATIC __inline int groupAll(struct u_lhead *E)
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



STATIC int buildLattice(c_ideal_t *edges, size_t edge_w, struct vertex *root, int n)
{
	int k;
	struct u_iterator i;
	struct u_lhead *E = _alloca(n * sizeof(*E)); 
	
	memset(E, 0, n * sizeof *E);

	GUARD(push_children(edges, edge_w, root));
	GUARD(groupAll(E));

	for (k = n; k >= 1; k--)
	{
		FOR_X_IN_LIST_REVERSE(i, &E[k-1])
			GUARD(process(edges, edge_w, k, UL_X(i)));

		FOR_X_IN_LIST_REVERSE(i, &E[k-1])
		{
			struct vertex *v = UL_X(i);
			delChild(v->parent, v);
		}
	}

	return G_SUCCESS;
}




///// top level wrappers /////


int idealLattice(c_ideal_t p_relations[][2], size_t p_reln, size_t n, struct ideal_lattice *lattice)
{
	struct ctx x;
	struct vertex *root;
	c_index_t *neighbors;
	c_ideal_t     *edges;
	size_t i, slen, result;
	struct p_iterator p;

	ctx_init(&x, p_relations, p_reln, n);

	root      = Left(&x, n);
	slen      = x.vertex_count * x.max_neighbors;
	edges     = malloc(slen * sizeof *edges);
	neighbors = malloc(slen * sizeof *neighbors);

	if (buildLattice(edges, x.max_neighbors, root, n) != G_SUCCESS)
	{
		free(edges);
		free(neighbors);
		ctx_free(&x);
		return G_ERROR;
	}

	FOR_X_IN_POOLS(p, P_ALLOC_VERTEX)
	{
		struct vertex *v = P_X(p);
		struct u_iterator j;
		size_t k;

		i = p.global_index * x.max_neighbors;

		FOR_X_IN_LIST_REVERSE(j, &v->impred)
			neighbors[i + UL_N(j)] = UL_X(j)->index;

		for (k = UL_N(j); k < x.max_neighbors; ++k)
			neighbors[i + k] = 0;
	}

	for (i = 0; i < slen; ++i)
		edges[i] = x.linear_extension[edges[i] - 1];
	
	lattice->ideals           = edges;
	lattice->neighbors        = neighbors;
	lattice->source           = root->index;
	lattice->sink             = n;
	lattice->vertex_count     = x.vertex_count;
	lattice->max_neighbors    = x.max_neighbors;
	lattice->extension_length = n;

	for (i = 0; i < x.max_neighbors; ++i)
		assert(neighbors[INDEX2(x.max_neighbors, lattice->sink, i)] == 0);

	ctx_free(&x);
	return G_SUCCESS;
}

