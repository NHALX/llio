#ifndef __OPENCL_VERSION__ 
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

//#include <Windows.h>

#include <time.h>

#ifdef __OSX__
#include <OpenCL/cl.h>
#else
#include <CL/cl.h>
#endif

#include "le_test.h"
#include "../Common/ll_formulas.h"
#include "../../Database/database.h"

#include "../Common/OpenCLDebugHack.h"

void init_all(struct graph *g)
{
	init_graph(g, vertices, sizeof(vertices) / sizeof(*vertices), (cl_uchar*)adjacency, ADJACENCY_MAX, LE_SIZE, node2id, IDMAP_LEN);
	init_count(g);
}


extern cl_result_t * gpu_LE(struct gpu_context *ctx, struct graph *g, cl_float *cfg_input, size_t * result_n);
extern void * gpu_init(char *, char *);

//#define DEBUG_ON_CPU
/*
Starting: 20,10
[0]={1061,7}
[1]={0,0}
*/

int main1()
{
	//struct cl_string_ctx le;
	void *gpu_ctx;
	int i;
	cl_ulong n;
	cl_result_t *results;
	cl_float cfg[CFG_SIZE] = { 3.0f, 3.4f, 100.0f, 18.0f, 15000.0f };
	//cl_uchar le_buf[COMBO_LEN];

	struct graph g;
	cl_result_t maximum = { -1, -1 };

#ifdef DEBUG_ON_CPU
	int global_size, local_size;
	cl_result_t *scratch;
#endif
	int result_n;

	init_all(&g);

	//le.s_index = 0;
	///le.s_len = COMBO_LEN;
	//le.s_ptr = le_buf;

	n = g.counts[1]; // TODO: make sure this is a valid assumption

#ifdef DEBUG_ON_CPU
	global_size = n;
	local_size = 1;// global_size / 2;
	result_n    = n / local_size;

	results = calloc(result_n, sizeof(*results));
	scratch = calloc(local_size, sizeof(*results));

	MAINLOOP(i, kernel_LE(db_items, db_passives, cfg, node2id, g.masks, g.counts, g.adjacency, g.max_neighbors, g.combo_len, scratch, results), global_size, local_size);
#else
	gpu_ctx = gpu_init("D:/GitRoot/llio/Source/Optimizer/BuildPath/genLE_Kernel.c", "-DUSE_OPENCL -ID:/GitRoot/llio/Source/Optimizer/BuildPath -ID:/GitRoot/llio/Source/Optimizer/Libs/Random123-1.08/include/");
	results = gpu_LE(gpu_ctx, &g, cfg, &result_n);
#endif

	for (i = 0; i < result_n; ++i)
	{
		printf("[%d]={%f,%d}\n", i, results[i].metric, results[i].index);
		if (maximum.metric < results[i].metric)
			maximum = results[i];
			
	}

	printf("max=%f,i=%d\n", maximum.metric, maximum.index);
	

	free(results);

#ifdef DEBUG_ON_CPU
	free(scratch);
#else
	free(gpu_ctx);
#endif
}

#endif


/*
struct vertex {
int label;
struct vertex *parent;
struct vertex *left;
struct vertex *right;
};

struct vertex * vertex_new()
{
struct vertex * x = (struct vertex *) calloc(1, sizeof (struct vertex));
if (!x)
abort();

return x;
}

struct vertex * vertex_dup(struct vertex *x)
{
struct vertex *y = (struct vertex *) calloc(1, sizeof (struct vertex));
if (!y)
abort();

if (x)
memcpy(y,x,sizeof(*y));

return y;
}



int ImPredTest(int k, struct vertex *children)
{

}


void right(int i, struct vertex *r, struct vertex *root)
{
int index;
#define CHILD_N 2
struct vertex *children[CHILD_N];

if (r->left && r->right && (r->left->label < r->right->label))
{
children[0] = r->left;
children[1] = r->right;
} else
{
children[1] = r->left;
children[0] = r->right;
}

for (index = 0; index < CHILD_N; ++index)
{
struct vertex *s,*t;

if (children[index] == NULL || ImPredTest(s->label, children))
continue;

s = children[index];
t = vertex_dup(s);

t->parent = root;
root->left = t;
t->label = s->label;
right(i,s,t);
}
}

struct vertex * left(int i)
{
struct vertex *root = vertex_new();
struct vertex *r;

if (i == 0)
return root;

r = left(i-1);
right(i,r,root);

r->parent  = root;
root->left = r;
r->label   = i;

return root;
}

char *
toString(struct vertex *v)
{
char *l,*r,*buffer;

l = (v->left)  ? toString(v->left) : "{}";
r = (v->right) ? toString(v->right) : "{}";

buffer = malloc(strlen(l)+strlen(r)+128);

sprintf(buffer, "{%d,%s,%s,%p}", v->label, l, r, v->parent);
return buffer; // leaks memory
}

void main()
{
struct vertex *v = left(1);
printf("%s\n", toString(v));
}
*/
