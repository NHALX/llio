#ifdef __OSX__
#include <OpenCL/cl.h>
#else
#include <CL/cl.h>
#endif

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>

#include "../le.h"
#include "../genLE_Init.h"

#define LATTICE_N 6



void print_tree(struct ctx *x, struct vertex *v)
{
	struct u_iterator i;

	FOR_X_IN_LIST(i, &v->children)
	{
		struct vertex *v2 = UL_X(i);
		printf("(\"%d::%p\" -> \"%d::%p\"), \n", v->label, v, v2->label, v2);
		print_tree(x, v2);
	}

}

void show_mask(struct ctx *x, c_mask_t mask, char *buf)
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
			n = x->linear_extension[n - 1];

			itoa(n, number, 10);
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

void print_final(struct ctx *x, unsigned char *visited, c_ideal_t *edges, struct vertex *v, c_mask_t le)
{
	c_mask_t next;
	size_t max_mask_len = (x->adjacency_dim * 2) + 3;
	char *s1 = _alloca(max_mask_len);
	char *s2 = _alloca(max_mask_len);
	struct u_iterator j;

	show_mask(x, le, s1);

	FOR_X_IN_LIST_REVERSE(j, &v->impred)
	{
		size_t i        = (v->index*x->max_neighbors) + UL_N(j);
		c_ideal_t ideal = edges[i];

		if (visited[i] == 0)
		{
			visited[i] = 1;

			next = le & ~(1 << (ideal - 1));
			show_mask(x, next, s2);

			printf("{%s -> %s, %d},\n", s1, s2, x->linear_extension[ideal - 1]);
			print_final(x, visited, edges, UL_X(j), next);
		}
	}
}


void print_lattice(struct ctx *x, c_ideal_t *edges)
{
	struct u_iterator j;
	struct p_iterator p;

	FOR_X_IN_POOLS(p, P_ALLOC_VERTEX)
	{	
		struct vertex *v = P_X(p);

		FOR_X_IN_LIST_REVERSE(j, &v->impred)
		{
			struct vertex *c = UL_X(j);
			c_ideal_t ideal = edges[INDEX2(x->max_neighbors, v->index, UL_N(j))];

			printf("{\"%d::%p\" -> \"%d::%p\", %d}, \n", v->label, v, c->label, c, ideal);
		}
	}

}

void print_rest(struct ctx *x, struct vertex *v, c_ideal_t *edges)
{
	size_t output_len = x->vertex_count*x->max_neighbors;
	c_mask_t le = 0xffffffff >> (32 - LATTICE_N);
	unsigned char *visited;


	visited = calloc(output_len, sizeof (*visited));

	if (visited == NULL)
	{
		printf("error: %s:%d. malloc failure.\n", __FILE__, __LINE__);
		exit(-1);
	}

	printf("ideallattice = {\n");
	print_lattice(x, edges);
	printf("};\n");

	memset(visited, 0, output_len*sizeof(*visited));

	printf("latticeEdges = {\n");
	print_final(x, (unsigned char*)visited, edges, v, le);
	printf("};\n");

	free(visited);
}



#define PRINT

#ifdef PRINT
#define SIMULATION_RUNS 1
#else
#define SIMULATION_RUNS 100000000
#endif

//	c_ideal_t *le        = _alloca(sizeof *le * il.extension_length);
c_ideal_t le[128];
size_t le_n = 0;

void
all_extensions(struct ideal_lattice il, c_index_t index, size_t le_n)
{
	size_t j,i = il.max_neighbors*index;

	c_index_t *neighbors = &il.neighbors[i];
	c_ideal_t *ideals    = &il.ideals[i];

	if (le_n >= il.extension_length)
	{
		printf("{");
		for (j = il.extension_length; j > 0; --j)
			printf("%d,", le[j-1]);
		printf("}\n");

		return;
	}
	
	for (j = 0; j < il.max_neighbors; ++j)
	{
		if (neighbors[j] == 0)
			continue;

		le[le_n] = ideals[j];
		all_extensions(il, neighbors[j], le_n + 1);
	}
	
}

//#define BENCHMARK

int
main()
{
	size_t i;
	struct ideal_lattice lattice;
	c_ideal_t poset[][2] = {
		{ 3, 1 },
		{ 5, 4 },
		{ 6, 4 },
		{ 1, 2 },
		{ 4, 2 }
	};

	__glbinit__lattice();

#ifdef BENCHMARK
	idealLattice(poset, 5, 17, &lattice);
#else
	idealLattice(poset, 5, 6, &lattice);

	for (i = 0; i < lattice.vertex_count; ++i)
	{
		size_t base = i * lattice.max_neighbors;
		size_t j;
		
		for (j = 0; j < lattice.max_neighbors; ++j)
			if (lattice.neighbors[base + j] != 0)
				printf("{%d -> %d,%d},\n", lattice.neighbors[base + j], i, lattice.ideals[base + j]);
	}

	all_extensions(lattice, lattice.source, 0);
#endif

}


int
main3()
{
	struct vertex *v;
	struct ctx x;
	int i;
	size_t edgelen = 0;
	c_ideal_t *edges = NULL;
	c_ideal_t poset[][2] = {
		{ 3, 1 },
		{ 5, 4 },
		{ 6, 4 },
		{ 1, 2 },
		{ 4, 2 }
	};

#ifndef NDEBUG	
	unittest_u_list();
#endif


	ctx_init(&x, poset, 5, 6);

	for (i = 0; i < SIMULATION_RUNS; ++i)
	{
		v = Left(&x, LATTICE_N);

		if (edges == NULL)
		{
			edgelen = x.vertex_count * x.max_neighbors * sizeof *edges;
			edges   = calloc(edgelen, 1);
		}

#ifdef PRINT
		printf("idealtree = {\n");
		print_tree(&x, v);
		printf("};\n");
#endif
		GUARD(buildLattice(edges, x.max_neighbors, v, LATTICE_N));

#ifdef PRINT
		print_rest(&x, v, edges);
#endif
		// reset
		x.vertex_count  = 0;
		x.max_neighbors = 0;
		p_release(P_ALLOC_ULIST);
		p_release(P_ALLOC_VERTEX);
	}

	if (edges)
		free(edges);

	return 0;
}
