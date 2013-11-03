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
typedef cl_uint c_mask_t;

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

#define LATTICE_N 6


STATIC void Right(struct ctx *x, int index, struct vertex *r, struct vertex *root)
{
	struct slist *node;
	
	if (r->children_len)
	{
		struct vertex **set = _alloca(r->children_len * sizeof(*set));
		size_t i            = r->children_len - 1;

		for (node = r->children; node; node = node->next) 
			set[i--] = node->value;

		for (i = 0; i < r->children_len; ++i)
		{
			struct vertex *s = set[i];
			struct vertex *t;

			static int adjacency[6][6] = {
				{ 0, 1, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0 },
				{ 1, 0, 0, 0, 0, 0 },
				{ 0, 1, 0, 0, 0, 0 },
				{ 0, 0, 0, 1, 0, 0 },
				{ 0, 0, 0, 1, 0, 0 }
			};
			static int map[6] = { 5, 3, 1, 6, 4, 2 };
			
			if (adjacency[map[s->label - 1] - 1][map[index - 1] - 1])
				continue;

			t = vertex(x);
			t->parent = root;
			addChild(x, root, t);
			t->label = s->label;
			Right(x, index, s, t);
		}
	}
}


STATIC struct vertex * Left(struct ctx *x, int i)
{
	struct vertex * root = vertex(x);
	struct vertex * r;

	if (i == 0)
		return root;
	
	r = Left(x, i - 1);

	Right(x,i,r,root);
	r->parent = root;
	addChild(x, root, r);
	r->label = i;
	return root;
}



////////////////////// hasse diagram of the lattice of ideals //////////////////////

STATIC void push_edge(struct ctx *x, struct vertex *v, c_ideal_t ideal)
{
	assert(ideal != 0);

	x->edges[(v->index*x->max_neighbors) + v->edge_len] = ideal;
	v->edge_len++;
}

STATIC void push_children(struct ctx *x, struct vertex *v) 
{
	struct slist *node;

	for (node = v->children; node; node = node->next)
	{
		slh_append(&v->impred, node->value); 
		push_edge(x, v, node->value->label);
	}
}


/*
STATIC void groupAll(struct ctx *x, struct slist_head *E, struct vertex *v) 
{
	struct slist *node;

	if (v->label != 0)
		slh_append(&E[v->label - 1], v); 

	for (node = v->children; node; node = node->next)
		groupAll(x, E, node->value);
}
*/

STATIC void groupAll2(struct ctx *x, struct slist_head *E) 
{
	size_t i;

	for (i = 0; i < v_alloc_index; ++i)
	{
		c_ideal_t label = v_alloc_storage[i].label;

		if (label != 0)
			slh_append(&E[label-1], &v_alloc_storage[i]);
	}
}


STATIC __inline void process(struct ctx *ctx, size_t k, struct vertex *v)
{
	struct slist *n;
	struct vertex *v2, *v3;
	size_t i;

	// OPTIMIZATION: This loop is a major hotspot. Accounts for ~2/3 of total execution time.
	for (
		n = v->parent->impred.first, i = 0;
		n && n->value != v;
		n = n->next, ++i)
	{
		v2 = n->value;
		v3 = v2->children->value;
		
		assert(v2->label > k && v->label == k);
		assert(v3->label == k);

		slh_append(&v->impred, v3);
		push_edge(ctx, v, EDGE(ctx, v->parent->index, i));
	}

	push_children(ctx, v);
}


STATIC void buildLattice(struct ctx *ctx, struct vertex *root, int n)
{
	int k;
	struct slist *node;
	struct slist_head *E = _alloca(n * sizeof(*E)); 
	
	memset(E, 0, n*sizeof(*E));
	push_children(ctx, root);

	groupAll2(ctx, E);

	for (k = n; k >= 1; k--)
	{
		for (node = E[k - 1].first; node != NULL; node = node->next)
			process(ctx, k, node->value);

		for (node = E[k - 1].first; node != NULL; node = node->next)
			delChild(node->value->parent, node->value);
	}
}




///// top level wrappers /////


void idealLattice(void *data, c_ideal_t n, c_index_t *start, c_ideal_t **outideal, c_index_t **outneighbors)
{
	struct ctx x;
	memset(&x, 0, sizeof x);
	struct vertex *root = Left(&x, n);

	x.edges = calloc(x.vertex_count*x.max_neighbors, sizeof (*x.edges));

	if (x.edges == NULL)
	{
		printf("error: %s:%d. malloc failure.\n", __FILE__, __LINE__);
		exit(-1);
	}

	buildLattice(&x, root, n);
	*outideal     = x.edges;
	//*outneighbors = x.ImPred; TODO: copy/convert preds
	*start        = root->index;
}

/////////////////// DEBUG ///////////////////



void print_tree(struct ctx *x, struct vertex *v)
{
	struct slist *node;

	for (node = v->children; node != NULL; node = node->next)
	{
		struct vertex *v2 = node->value;
		printf("(\"%d::%p\" -> \"%d::%p\"), \n", v->label, v, v2->label, v2);
		print_tree(x, v2);
	}

}

void show_mask(c_mask_t mask, char *buf)
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
			static int map[6] = { 5, 3, 1, 6, 4, 2 };
			int n = (j + 1);
			n = map[n-1];

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

void print_final(struct ctx *x, unsigned char *visited, struct vertex *v, c_mask_t le)
{
	c_mask_t next;
	char s1[256];
	char s2[256];
	struct slist *node;
	size_t i;

	show_mask(le, s1);


	for (i=0, node=v->impred.first; node; ++i, node=node->next)
	{
		c_ideal_t ideal = EDGE(x, v->index, i);

		static int map[6] = { 5, 3, 1, 6, 4, 2 }; // TODO: remove

		if (visited[(v->index*x->max_neighbors) + i])
			continue;
		else
			visited[(v->index*x->max_neighbors) + i] = 1;

		
		next = le & ~(1<<(ideal-1));
		show_mask(next, s2);

		printf("{%s -> %s, %d},\n", s1, s2, map[ideal-1]);
		print_final(x, visited, node->value, next);
	}
}

void print_lattice(struct ctx *x, struct vertex *root, unsigned char *visited, struct vertex *v)
{
	size_t i;
	struct slist *node;

	for (i=0, node=v->impred.first; node; ++i, node=node->next)
	{
		if (visited[(v->index*x->max_neighbors) + i])
			continue;
		else
			visited[(v->index*x->max_neighbors) + i] = 1;

		printf("{\"%d::%p\" -> \"%d::%p\", %d}, \n", v->label, v, node->value->label, node->value, EDGE(x, v->index, i));
		print_lattice(x, root, visited, node->value);
	}
}

void print_rest(struct ctx *x, struct vertex *v)
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
	print_lattice(x, v, (unsigned char*)visited, v);
	printf("};\n");

	memset(visited, 0, output_len*sizeof(*visited));

	printf("latticeEdges = {\n");
	print_final(x, (unsigned char*)visited, v, le);
	printf("};\n");

	free(visited);
}



//#define PRINT

#ifdef PRINT
#define SIMULATION_RUNS 1
#else
#define SIMULATION_RUNS 100000000
#endif

void reset(struct ctx *x, size_t n)
{
	//memset(x->edges, 0, n);
	//memset(s_alloc_storage, 0, sizeof s_alloc_storage);
	//memset(v_alloc_storage, 0, sizeof v_alloc_storage);
	s_alloc_index    = 0;
	v_alloc_index    = 0;
	x->max_neighbors = 0;
	x->vertex_count  = 0;
}

int
main()
{
	struct vertex *v;
	struct ctx x;
	int i;
	size_t edgelen = 0;
	memset(&x, 0, sizeof x);

	for (i = 0; i < SIMULATION_RUNS; ++i)
	{
		v = Left(&x, LATTICE_N);

		if (x.edges == NULL)
		{
			edgelen = x.vertex_count*x.max_neighbors*sizeof (*x.edges);
			x.edges = calloc(edgelen,1);

			if (x.edges == NULL)
			{
				printf("error: %s:%d. malloc failure.\n", __FILE__, __LINE__);
				exit(-1);
			}
		}
#ifdef PRINT
		printf("idealtree = {\n");
		print_tree(&x, v);
		printf("};\n");
#endif
		buildLattice(&x, v, LATTICE_N);

#ifdef PRINT
		print_rest(&x, v);
#endif
		reset(&x, edgelen);
	}

	free(x.edges);
	return 0;
}

/*idealtree = {
   ("0::0094AFE8" -> "6::0094B0F4"),
   ("6::0094B0F4" -> "5::0094B200"),
   ("5::0094B200" -> "4::0094B30C"),
   ("4::0094B30C" -> "3::0094B418"),
   ("3::0094B418" -> "2::0094B524"),
   ("2::0094B524" -> "1::0094B630"),
   ("3::0094B418" -> "1::0094B73C"),
   ("4::0094B30C" -> "1::0094B848"),
   ("5::0094B200" -> "3::0094BA60"),
   ("3::0094BA60" -> "2::0094BC78"),
   ("2::0094BC78" -> "1::0094BD84"),
   ("3::0094BA60" -> "1::0094BB6C"),
   ("5::0094B200" -> "1::0094B954"),
   ("6::0094B0F4" -> "3::0094BE90"),
   ("3::0094BE90" -> "2::0094BF9C")
   };
idealtree2 = {
   ("0::007FA010" -> "6::007FA068"),
   ("6::007FA068" -> "5::007FA0C0"),
   ("5::007FA0C0" -> "4::007FA118"),
   ("4::007FA118" -> "3::007FA170"),
   ("3::007FA170" -> "2::007FA1C8"),
   ("2::007FA1C8" -> "1::007FA220"),
   ("3::007FA170" -> "1::007FA278"),
   ("4::007FA118" -> "1::007FA2D0"),
   ("5::007FA0C0" -> "3::007FA458"),
   ("3::007FA458" -> "2::007FA5E0"),
   ("2::007FA5E0" -> "1::007FAFE8"),
   ("3::007FA458" -> "1::007FA540"),
   ("5::007FA0C0" -> "1::007FA3B8"),
   ("6::007FA068" -> "3::007FB088"),
   ("3::007FB088" -> "2::007FB170")
   };

ideallattice = {
   {"0::0094AFE8" -> "6::0094B0F4", 6},
   {"6::0094B0F4" -> "5::0094B200", 5},
   {"5::0094B200" -> "4::0094B30C", 4},
   {"4::0094B30C" -> "3::0094B418", 3},
   {"3::0094B418" -> "2::0094B524", 2},
   {"2::0094B524" -> "1::0094B630", 1},
   {"3::0094B418" -> "1::0094B73C", 1},
   {"1::0094B73C" -> "1::0094B630", 2},
   {"4::0094B30C" -> "1::0094B848", 1},
   {"1::0094B848" -> "1::0094B73C", 3},
   {"5::0094B200" -> "3::0094BA60", 3},
   {"3::0094BA60" -> "3::0094B418", 4},
   {"3::0094BA60" -> "2::0094BC78", 2},
   {"2::0094BC78" -> "2::0094B524", 4},
   {"2::0094BC78" -> "1::0094BD84", 1},
   {"1::0094BD84" -> "1::0094B630", 4},
   {"3::0094BA60" -> "1::0094BB6C", 1},
   {"1::0094BB6C" -> "1::0094B73C", 4},
   {"1::0094BB6C" -> "1::0094BD84", 2},
   {"5::0094B200" -> "1::0094B954", 1},
   {"1::0094B954" -> "1::0094B848", 4},
   {"1::0094B954" -> "1::0094BB6C", 3},
   {"6::0094B0F4" -> "3::0094BE90", 3},
   {"3::0094BE90" -> "3::0094BA60", 5},
   {"3::0094BE90" -> "2::0094BF9C", 2},
   {"2::0094BF9C" -> "2::0094BC78", 5}
   };
latticeEdges = {
   {{5, 3, 1, 6, 4, 2} -> {5, 3, 1, 6, 4}, 2},
   {{5, 3, 1, 6, 4} -> {5, 3, 1, 6}, 4},
   {{5, 3, 1, 6} -> {5, 3, 1}, 6},
   {{5, 3, 1} -> {5, 3}, 1},
   {{5, 3} -> {5}, 3},
   {{5} -> {}, 5},
   {{5, 3} -> {3}, 5},
   {{3} -> {}, 3},
   {{5, 3, 1} -> {3, 1}, 5},
   {{3, 1} -> {3}, 1},
   {{5, 3, 1, 6} -> {5, 3, 6}, 1},
   {{5, 3, 6} -> {5, 3}, 6},
   {{5, 3, 6} -> {5, 6}, 3},
   {{5, 6} -> {5}, 6},
   {{5, 6} -> {6}, 5},
   {{6} -> {}, 6},
   {{5, 3, 6} -> {3, 6}, 5},
   {{3, 6} -> {3}, 6},
   {{3, 6} -> {6}, 3},
   {{5, 3, 1, 6} -> {3, 1, 6}, 5},
   {{3, 1, 6} -> {3, 1}, 6},
   {{3, 1, 6} -> {3, 6}, 1},
   {{5, 3, 1, 6, 4} -> {5, 3, 6, 4}, 1},
   {{5, 3, 6, 4} -> {5, 3, 6}, 4},
   {{5, 3, 6, 4} -> {5, 6, 4}, 3},
   {{5, 6, 4} -> {5, 6}, 4}
   };
   */