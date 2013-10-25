#include "Random123/philox.h"
#include "le.h"


void LE_random(philox2x32_ctr_t *rand_c, philox2x32_key_t rand_k, __constant struct vertex *vertex, mask_t end,struct string_ctx le);


void print_le(struct string_ctx s);

#define QUIET
	
static 
unsigned char
set_difference(mask_t a, mask_t b, int le_n) // PORTING: possible endian issues
{
	int j;
	unsigned char ideal = 0;
	mask_t mask = a & ~b;
	

	for (j = 0; j < le_n; j++)
	{
		if (1<<j & mask)
		{
			ideal = j+1;
			break;
		}
	}

	return ideal;
}



static
unsigned char
extend(mask_t a, mask_t b, struct string_ctx *s)
{
	unsigned char ideal;
	
	if (a == b)
		return 0;

	ideal = set_difference(a, b, s->s_len);

	if (ideal != 0 && s->s_index < s->s_len)
	{
		s->s_ptr[s->s_index] = ideal;
		s->s_index++;
	}
	return ideal;
}



void
print_le(struct string_ctx s)
{
#ifndef QUIET
	unsigned int i;

	putchar('{');
	for (i = 0; i < s.s_len; ++i)
	{
		DEBUG_PRINTF("%d", s.s_ptr[i]);
		
		if (i+1 < s.s_len)
			putchar(',');
	}

	putchar('}');
	putchar('\n'); 
#endif
}

#define DFS_MAX_STACKLEN (COMBO_LEN*(MAX_NEIGHBORS-1))



__kernel void genWalkLE_NR_kernel(__constant struct vertex *vertex, unsigned int max_depth)
{
	int i, si = 1;
	struct stack_context nodes_to_visit[DFS_MAX_STACKLEN+1];
	struct stack_context ctx;
	struct string_ctx le;
	unsigned char le_buf[COMBO_LEN];

	le.s_index = 0;
	le.s_len   = COMBO_LEN;
	le.s_ptr   = le_buf;

	nodes_to_visit[0].node      = &vertex[1];
	nodes_to_visit[0].prev_mask = 0;
	nodes_to_visit[0].le_index  = 0;

	
	while (si > 0) 
	{
		ctx = nodes_to_visit[--si];
		le.s_index = ctx.le_index;
	//	printf("v=%d\n", ctx.prev_mask);
	//	printf("le_index=%d\n", le.s_index);

		extend(ctx.node->mask,ctx.prev_mask,&le);
		
		if (le.s_index >= le.s_len)//(ctx.node->mask == graph->end)
		{	
			//le.s_len = max_depth;
			DEBUG_PRINTF("count: %llu\n", ctx.node->count);
            printf("{%d,%d,%d,%d,%d}\n", le_buf[0], le_buf[1], le_buf[2], le_buf[3], le_buf[4]);
			print_le(le);
		}
		else
		{
			struct stack_context x;

			for (i = MAX_NEIGHBORS - 1; i >= 0 ; --i)
			{
				if (ctx.node->neighbors_out[i] == 0)
					continue;
			
				if (si+1 > DFS_MAX_STACKLEN)
				{
					DEBUG_PRINTF("SI=1>combolen:%d,%d\n", COMBO_LEN, si);
					return;
				}
			
				x.node      = &vertex[ctx.node->neighbors_out[i]];
				x.prev_mask = ctx.node->mask;
				x.le_index  = le.s_index;

				nodes_to_visit[si++] = x;
			}
		}

	}

}


/*
__kernel void genWalkLE_NR_kernel(__constant struct vertex *vertex, unsigned int max_depth)
{
	int i, si = 1;
	struct stack_context nodes_to_visit[DFS_MAX_STACKLEN+1];
	struct stack_context ctx;
	struct string_ctx le;
	unsigned char le_buf[COMBO_LEN];
    
	le.s_index = 0;
	le.s_len   = COMBO_LEN;
	le.s_ptr   = le_buf;
    
	nodes_to_visit[0].node      = &vertex[1];
	nodes_to_visit[0].prev_mask = 0;
	nodes_to_visit[0].le_index  = 0;
    
	
	while (si > 0)
	{
		ctx = nodes_to_visit[--si];
		le.s_index = ctx.le_index;
        //	printf("v=%d\n", ctx.prev_mask);
        //	printf("le_index=%d\n", le.s_index);
        
		extend(ctx.node->mask,ctx.prev_mask,&le);
		
		if (le.s_index >= max_depth)//(ctx.node->mask == graph->end)
		{
			//le.s_len = max_depth;
			DEBUG_PRINTF("count: %llu\n", ctx.node->count);
            printf("{%d,%d,%d,%d,%d}\n", le_buf[0], le_buf[1], le_buf[2], le_buf[3], le_buf[4]);
			print_le(le);
		}
		else
		{
			struct stack_context x;
            
			for (i = MAX_NEIGHBORS - 1; i >= 0 ; --i)
			{
				if (ctx.node->neighbors_out[i] == 0)
					continue;
                
				if (si+1 > DFS_MAX_STACKLEN)
				{
					DEBUG_PRINTF("SI=1>combolen:%d,%d\n", COMBO_LEN, si);
					return;
				}
                
				x.node      = &vertex[ctx.node->neighbors_out[i]];
				x.prev_mask = ctx.node->mask;
				x.le_index  = le.s_index;
                
				nodes_to_visit[si++] = x;
			}
		}
        
	}
    
}
*/




void LE_random(
	philox2x32_ctr_t *rand_c,
	philox2x32_key_t rand_k, 
	__constant struct vertex *vertex, 
	mask_t end,
	struct string_ctx le)
{
	philox2x32_ctr_t rv;
	int i;
	__constant struct vertex *v = &vertex[1];
	__constant struct vertex *v2;
	uint64_t t;
	uint64_t c;
	union { 
		uint64_t r64;
		uint32_t r32[2];
	} r;

	while (v->mask != end)
	{
		t = v->count;
		c = 0;

	    rand_c->v[1] += 1;      /* another loop-dependent application variable */
	    rv       = philox2x32(*rand_c, rand_k);
		r.r32[0] = rv.v[0];
		r.r32[1] = rv.v[1];
		r.r64    = 1 + (r.r64) % t;
	
		for (i = 0; i < MAX_NEIGHBORS; ++i)
		{
			if (v->neighbors_out[i] == 0)
				continue;
			
			v2 = &vertex[v->neighbors_out[i]];
			c += v2->count;

			if (r.r64 <= c)
			{
				extend(v2->mask, v->mask, &le);
				v = v2;
				break;
			}
		}
	}
}

__kernel void kernel_LE_random(
	philox2x32_key_t rand_k, 
	__constant struct vertex *vertex,
	mask_t end, 
	ulong nsamples)
{
	philox2x32_ctr_t rand_c;
	struct string_ctx le;
	unsigned char le_buf[COMBO_LEN];
	unsigned int i;

	le.s_index = 0;
	le.s_len   = COMBO_LEN;
	le.s_ptr   = le_buf;

	rand_c.v[0] = get_global_id(1); //v->mask; /* some loop-dependent application variable */
	rand_c.v[1] = 0;

	for (i = 0; i < nsamples; ++i)
		LE_random(&rand_c, rand_k, vertex, end, le);
}







#ifndef __OPENCL_VERSION__ 
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

//#include <Windows.h>

#include <time.h>

#include "le_test.h"


uint64_t assign(const struct graph *graph, __constant struct vertex *vertex)
{
	int i;
	uint64_t lef = 0;
	__constant struct vertex *v2;

	vertex->visited = 1;

	for (i = 0; i < MAX_NEIGHBORS; ++i)
	{
		if (vertex->neighbors_out[i] == 0)
			continue;

		v2 = &graph->vertex[vertex->neighbors_out[i]];

		if (v2->mask == graph->end)
			lef = 1;

		else if (v2->visited)
			lef += v2->count;

		else
			lef += assign(graph,v2);
	}

	vertex->count = lef;
	return lef;
}

void AssignWrapper(struct graph *g)
{
	assign(g, &g->vertex[1]);
	g->end_v->count = 1;
}


void init_all(struct graph *g, philox2x32_key_t *rand_k)
{
	//LARGE_INTEGER t1,t2;
    if (rand_k)
    {
        memset(rand_k, 0, sizeof *rand_k);
        rand_k->v[0] = (unsigned int) clock();
    }
	init_graph(g, vertices, sizeof(vertices)/sizeof(struct vertex));

	//QueryPerformanceCounter(&t1);
	AssignWrapper(g);
	//QueryPerformanceCounter(&t2);
	//DEBUG_PRINTF("count=%d, time=%d\n", g->vertex[1].count, t2.QuadPart-t1.QuadPart);
}

uint64_t samples_needed(struct graph *g, double precision)
{
#define EULAR_GAMMA       0.5772156649015328606065120900824024310422
#define PRECISION(P)     (logl(1/logl(1/(P))))
#define SAMPLE_SIZE(N,P) ((N) * logl(N) + (N) * PRECISION(P))

	return ceill(SAMPLE_SIZE((double)g->vertex[1].count, precision));
}


/*
void genWalkLE_NR(struct graph *graph, struct string_ctx le)
{
	
	struct stack_context nodes_to_visit[DFS_MAX_STACKLEN+1];
	struct stack_context ctx;

	int i, si = 1;
	nodes_to_visit[0].node      = &graph->vertex[1];
	nodes_to_visit[0].prev_mask = 0;
	nodes_to_visit[0].le_index  = 0;

	
	while (si > 0) 
	{
		ctx = nodes_to_visit[--si];
		le.s_index = ctx.le_index;
	//	printf("v=%d\n", ctx.prev_mask);
	//	printf("le_index=%d\n", le.s_index);

		extend(ctx.node->mask,ctx.prev_mask,&le);
		
		if (le.s_index >= (COMBO_LEN/2)-1)//(ctx.node->mask == graph->end)
		{	
			le.s_len = (COMBO_LEN/2)-1;
			print_le(le);
		}
		else
		{
			struct stack_context x;

			for (i = MAX_NEIGHBORS - 1; i >= 0 ; --i)
			{
				if (ctx.node->neighbors_out[i] == 0)
					continue;
			
				if (si+1 > DFS_MAX_STACKLEN)
				{
					printf("SI=1>combolen:%d,%d\n", COMBO_LEN, si);
					exit(1);
				}
			
				x.node      = &graph->vertex[ctx.node->neighbors_out[i]];
				x.prev_mask = ctx.node->mask;
				x.le_index  = le.s_index;

				nodes_to_visit[si++] = x;
			}
		}

	}
	
}


void genWalkLE(const struct graph *graph, struct vertex *root, struct string_ctx le) // copy semantics wanted on le
{
	struct vertex *v;
	int i;

	for (i = 0; i < MAX_NEIGHBORS; ++i)
	{
		if (root->neighbors_out[i] == 0)
			continue;

		v = &graph->vertex[root->neighbors_out[i]];

		extend(v->mask,root->mask,&le);

		if (v->mask == graph->end){
			print_le(le);
		}
		else
			genWalkLE(graph, v, le);

		le.s_index--; // replace current slot on next iteration
	}
}
*/


extern void gpu_LERandom(void *ctx);
extern void gpu_LETree(void *ctx);
void * gpu_init(char *, char *);

int main()
{
	//LARGE_INTEGER t1,t2;
	philox2x32_key_t rand_k;
	struct string_ctx le;
	uint64_t nsamples;
	void *gpu_ctx;

	struct graph g;
	unsigned char le_buf[COMBO_LEN];

	
	init_all(&g, &rand_k);

	le.s_index = 0;
	le.s_len   = COMBO_LEN;
	le.s_ptr   = le_buf;
/*
	QueryPerformanceCounter(&t1);
	genWalkLE(&g, &g.vertex[1], le);
	QueryPerformanceCounter(&t2);
	printf("DEPTH FIRST SEARCH: %d\n", t2.QuadPart-t1.QuadPart);
*/

	//QueryPerformanceCounter(&t1);
	genWalkLE_NR_kernel(g.vertex, 2);
	//QueryPerformanceCounter(&t2);
	//DEBUG_PRINTF("ITERATIVE DEPTH FIRST SEARCH: %d\n", t2.QuadPart-t1.QuadPart);




	//genRandLE(&rand_c, &rand_k, &g, le);
	//print_le(le);
	

	nsamples = samples_needed(&g, 0.99);
	{
		unsigned char *set = (unsigned char*) calloc(COMBO_LEN, nsamples);
		//int filtered_count = 0;

		DEBUG_PRINTF("SAMPLE_SIZE=%llu\n",nsamples);
		le.s_index = 0;
		le.s_len   = COMBO_LEN;

		//QueryPerformanceCounter(&t1);
		// fill set
		/*
		for (i = 0; i < nsamples; ++i)
		{
			le.s_ptr = set+(COMBO_LEN*i);
			
		}*/
		kernel_LE_random(rand_k, g.vertex, g.end, nsamples);

		//QueryPerformanceCounter(&t2);
		//DEBUG_PRINTF("RANDOM SAMPLING: %d\n", t2.QuadPart-t1.QuadPart);

#ifdef FILTER_RANDOM
		// mark duplicates
		for (i = 0; i < nsamples; ++i)
		{
			int j, test = 0;
			unsigned char *ptr1 = set+(COMBO_LEN*i);
			
			for (j = 0; j < nsamples; ++j)
			{
				unsigned char *ptr2 = set+(COMBO_LEN*j);
				if (ptr1 != ptr2 &&	memcmp(ptr1, ptr2, COMBO_LEN) == 0)
					ptr2[0] = 0;
			}
		}

		filtered_count = 0;
		for (i = 0; i < nsamples; ++i)
		{
			le.s_ptr = set+(COMBO_LEN*i);

			if (*le.s_ptr != 0)
			{
				filtered_count++;
				print_le(le);
			}
		}
		
		DEBUG_PRINTF("filtered count: %d\n", filtered_count);
#endif

		free(set);
	}

	gpu_ctx = gpu_init(__FILE__, "-DUSE_OPENCL -I/Users/NHA/llio/Source/Optimizer/BuildPath -I/Users/NHA/llio/Source/Optimizer/Libs/Random123-1.08/include/");
	gpu_LERandom(gpu_ctx);
    gpu_LETree(gpu_ctx);
	free(gpu_ctx);
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
