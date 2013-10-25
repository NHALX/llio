#ifndef _LE_H_
#define _LE_H_

#ifndef __OPENCL_VERSION__ 
#include "../Common/OpenCLDebugHack.h"
#else
#define DEBUG_PRINTF(...) 
#endif

typedef unsigned int mask_t;


struct graph {
	mask_t end;
	__constant struct vertex *end_v;
	__constant struct vertex *vertex;
	unsigned int vertex_count;
};


struct string_ctx
{
	unsigned char *s_ptr;
	unsigned int   s_len;
	unsigned int   s_index;
};

struct stack_context 
{
	__constant struct vertex *node;
	mask_t prev_mask;
	unsigned int le_index;
};

//#define BIG


#ifdef BIG
#define MAX_NEIGHBORS 3
#define COMBO_LEN 9
#else
#define MAX_NEIGHBORS 2
#define COMBO_LEN 5
#endif


struct vertex {
	mask_t mask;
	unsigned short neighbors_out[MAX_NEIGHBORS];
	uint64_t count;
	unsigned char visited;
};


static inline void init_graph(struct graph *g, __constant struct vertex *vs, int len)
{
	g->end   = vs[len-1].mask;
	g->end_v = &vs[len-1];
	g->vertex = vs;
	g->vertex_count = len;
}

#endif
