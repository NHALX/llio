#ifdef __OSX__
#include <OpenCL/cl.h>
#else
#include <CL/cl.h>
#endif

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "le.h"

////////////////////// count data //////////////////////

#define GET_NEIGHBORS(P,S,I) (P + (I*S))

cl_ulong assign_count(const struct graph *graph, cl_uchar *visited, cl_uchar vertex_index)
{
	unsigned int i;
	cl_ulong lef = 0;
	cl_uchar *neighbors = GET_NEIGHBORS(graph->adjacency, graph->max_neighbors, vertex_index);
	cl_uchar next;

	visited[vertex_index] = 1;

	for (i = 0; i < graph->max_neighbors; ++i)
	{
		if (neighbors[i] == 0)
			continue;

		next = neighbors[i];
		
		if (graph->masks[next] == graph->end)
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

////////////////////// stack //////////////////////
#define BULLSHIT_STACK_SIZE 64

struct stack 
{
	struct vertex *base[BULLSHIT_STACK_SIZE];
	int length;
};

void stack_init(struct stack *s)
{ 
	memset(s, 0x0, sizeof (*s)); 
}

struct vertex *
stack_last(struct stack *s)
{
	if (s->length > 0)
		return s->base[s->length - 1];
	else
		abort();
}

void
stack_push(struct stack *s, struct vertex *value)
{
	s->base[s->length++] = value;
}

#define STACK_FORALL(I,STACK) for (I = 0; (I) < (STACK).length; ++(I))
#define STACK_FORALL_REVERSE(I,STACK) for (I = (STACK).length-1; (I) >= 0; --(I))

////////////////////// tree of ideals //////////////////////

struct vertex 
{
	int label;

	struct vertex *parent;
	struct stack   children;
	struct stack   ImPred;
	struct stack   edges;
	struct stack   visited;
};


struct vertex * vertex()
{
	struct vertex *v;
	v = calloc(1, sizeof(*v));
	stack_init(&v->ImPred);
	stack_init(&v->children);
	stack_init(&v->edges);
	return v;
}



void addChild(struct vertex *p, struct vertex *c)
{
	stack_push(&p->children, c);
}

void delChild(struct vertex *p, struct vertex *c)
{
	struct stack temp;
	int i;

	memcpy(&temp, &p->children, sizeof(temp));

	stack_init(&p->children);

	STACK_FORALL(i, temp)
	{
		struct vertex *v = temp.base[i];
		if (v != c)
			stack_push(&p->children, temp.base[i]);
	}
}

/*
int
isImPred(struct vertex *s, int i)
{
	int a, b;
	a = isImPred1(s, i);
	b = isImPred2(s, i);
	assert(a == b);
	return a;
}
*/

int
isImPred(struct vertex *s, int i)
{
	static int adjacency[6][6] = { { 0, 1, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0 }, { 1, 0, 0, 0, 0, 0 }, { 0, 1, 0,
		0, 0, 0 }, { 0, 0, 0, 1, 0, 0 }, { 0, 0, 0, 1, 0, 0 } };

	static int map[6] = {5, 3, 1, 6, 4, 2};
	
	return adjacency[map[s->label - 1] - 1][map[i - 1] - 1];
}

/*
int
isImPred1(struct vertex *s, int i)
{


	if (i == 2 || i == 1 || i == 4)
		return 0;

	else if (i == 3 && s->label == 1)
		return 1;

	else if (i == 3 && s->label == 2)
		return 1;

	else if (i == 5 && s->label == 4)
		return 1;

	else if (i == 6 && s->label == 5)
		return 1;

	else if (i == 6 && s->label == 3)
		return 1;

	else
		return 0;

}
*/

void Right(int index, struct vertex *r, struct vertex *root)
{
	int i;
	struct vertex *s;
	struct vertex *t;

	//for (i = 0; i < r->children.length; ++i)
	STACK_FORALL(i, r->children)
	{
		s = r->children.base[i];

		if (isImPred(s, index))
			continue;

		t = vertex();
		memcpy(t, s, sizeof(*t));
		t->children.length = 0;
		memset(&t->children, 0x0, sizeof(t->children));
		t->parent = root;
		addChild(root, t);
		t->label = s->label;
		Right(index, s, t);
	}
}

struct vertex * Left(int i)
{
	struct vertex * root = vertex();
	struct vertex * r;

	if (i == 0)
		return root;
	
	r = Left(i - 1);
	Right(i,r,root);
	r->parent = root;
	addChild(root, r);
	r->label = i;
	return root;
}



////////////////////// hasse diagram of the lattice of ideals //////////////////////


// TODO: check malloc failure
void findAll(struct vertex *v, int k, struct stack *s)
{
	struct vertex *v2;
	int i;

	if (v->label == k)
		stack_push(s, v);

	//for (i = 0; i < v->children.length; i++)
	STACK_FORALL_REVERSE(i, v->children)
	{
		v2 = v->children.base[i];
		findAll(v2, k, s);
	}
}

void
push_children(struct vertex *v)
{
	int i;
	//for (i = 0; i < v->children.length; ++i)
	STACK_FORALL_REVERSE(i, v->children)
	{
		stack_push(&v->ImPred, v->children.base[i]);
		stack_push(&v->edges, v->children.base[i]->label);
	}
}

void buildLattice(struct vertex *Tp, int n)
{
	int i,k;
	struct stack *E = calloc(n,sizeof(*E));

	push_children(Tp);

	for (k = 1; k <= n; k++)
		findAll(Tp, k, &E[k-1]);

	for (k = n; k >= 1; k--)
	{
		//for (i = 0; i < E[k].length; ++i)
		STACK_FORALL(i, E[k-1])
		{
			int j;
			struct vertex *v2, *v3;
			struct vertex *v = E[k-1].base[i];

			v->ImPred.length = 0;
			
			for (
				j=0, v2 = v->parent->ImPred.base[j]; 
				v2 != v; 
				++j, v2 = v->parent->ImPred.base[j])
			{
				int h, found = 0;
				v3 = stack_last(&v2->children);

				assert(v2->label > k && v->label == k);
				assert(v3->label == k);
				stack_push(&v->ImPred, v3);

				STACK_FORALL(h, v->parent->ImPred)
				{
					if (v->parent->ImPred.base[h] != v2)
						continue;

					assert(v->parent->ImPred.base[h] == v2);
					assert(v->parent->edges.base[h] != 0);

					found = v->parent->edges.base[h];
				}

				assert(found != 0);
				stack_push(&v->edges, found);
			}

			push_children(v);
		}

		STACK_FORALL(i, E[k-1])
		{
			struct vertex *v = E[k-1].base[i];
			delChild(v->parent, v);
		}
	}
}

////



void print_tree(struct vertex *v)
{
	struct vertex *v2;
	int i;

	STACK_FORALL_REVERSE(i, v->children)
	{
		v2 = v->children.base[i];
		printf("(\"%d::%p\" -> \"%d::%p\"), \n", v->label, v, v2->label, v2);
		print_tree(v2);
	}

}

void reset_visited(struct vertex *v)
{
	int i;

	STACK_FORALL(i, v->ImPred)
	{
		v->visited.base[i] = 0;
		reset_visited(v->ImPred.base[i]);
	}
	
}

void print_pred2(struct vertex *v)
{
	struct vertex *v2;
	int i;

	STACK_FORALL(i, v->ImPred)
	{
		if (v->visited.base[i])
			continue;
		else
			v->visited.base[i] = 1;

		v2 = v->ImPred.base[i];

		printf("{\"%d::%p\" -> \"%d::%p\", %d}, \n", v->label, v, v2->label, v2, v->edges.base[i]);
		print_pred2(v2);
	}
}


int mask(struct vertex *v, unsigned char *le_local, unsigned char *le, int max)
{
	int i, j;

	memset(le_local, 0, 5);

	for (j = i = 0; j < max; ++j)
	{
		if (le[j] != v->label && le[j] != 0)
			le_local[i++] = le[j];
	}

	return i;
}


void show_mask(cl_mask_t mask, char *buf)
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


void print_pred(struct vertex *v, cl_mask_t le)
{
	cl_mask_t next;
	struct vertex *v2;
	int i;
	int ideal;
	char s1[256];
	char s2[256];

	show_mask(le, s1);

	STACK_FORALL(i, v->ImPred)
	{
		static int map[6] = { 5, 3, 1, 6, 4, 2 };

		if (v->visited.base[i])
			continue;
		else
			v->visited.base[i] = 1;

		ideal = v->edges.base[i];
		next = le & ~(1<<(ideal-1));
		show_mask(next, s2);

		printf("{%s -> %s, %d},\n", s1, s2, map[ideal-1]);
		print_pred(v->ImPred.base[i], next);
	}
}


/* Helper macros */
#define HEX__(n) 0x##n##LU
#define B8__(x) ((x&0x0000000FLU)?1:0) \
	+ ((x & 0x000000F0LU) ? 2 : 0) \
	+ ((x & 0x00000F00LU) ? 4 : 0) \
	+ ((x & 0x0000F000LU) ? 8 : 0) \
	+ ((x & 0x000F0000LU) ? 16 : 0) \
	+ ((x & 0x00F00000LU) ? 32 : 0) \
	+ ((x & 0x0F000000LU) ? 64 : 0) \
	+ ((x & 0xF0000000LU) ? 128 : 0)

/* User macros */
#define B8(d) ((unsigned char)B8__(HEX__(d)))
#define B16(dmsb,dlsb) (((unsigned short)B8(dmsb)<<8) \
	+ B8(dlsb))
#define B32(dmsb,db2,db3,dlsb) (((unsigned long)B8(dmsb)<<24) \
	+ ((unsigned long)B8(db2) << 16) \
	+ ((unsigned long)B8(db3) << 8) \
	+ B8(dlsb))


#define LATTICE_N 6

void
main()
{
	unsigned char buf[1024];
	cl_mask_t le = B8(11111111) >> (8 - LATTICE_N);
	struct vertex *v = Left(LATTICE_N);

	printf("idealtree = {\n");
	print_tree(v);
	printf("};\n");

	buildLattice(v, LATTICE_N);


	printf("ideallattice = {\n");
	print_pred2(v);
	printf("};\n");

	reset_visited(v);

	printf("latticeEdges = {\n");
	print_pred(v, le);
	printf("};\n");
}