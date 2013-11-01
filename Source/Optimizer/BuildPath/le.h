#ifndef _LE_H_
#define _LE_H_


/*
#ifndef __OPENCL_VERSION__ 
#include "../Common/OpenCLDebugHack.h"
#else
#define DEBUG_PRINTF(...) 
#endif
*/

/*
#ifdef __OSX__
#include <OpenCL/cl.h>
#else
#include <CL/cl.h>
#endif
*/


//#pragma pack (push, 16)
typedef struct { cl_float metric; cl_ulong index; }    cl_result_t;
//#pragma pack(pop)
typedef cl_uint  cl_mask_t;

struct graph {
	cl_mask_t end;
	//struct cl_vertex *end_v;
	cl_mask_t        *masks;
	cl_ulong         *counts;
	cl_short         *idmap;
	unsigned int idmap_len;
	unsigned int vertex_count;
	cl_uchar         *adjacency;
	unsigned int max_neighbors;
	unsigned int combo_len;
};


static __inline void init_graph(
	struct graph *g, 
	cl_mask_t *ms, 
	unsigned int len, 
	cl_uchar *adjacency, 
	unsigned int max_neighbors, 
	unsigned int combo_len,
	cl_short *idmap,
	unsigned int idmap_len)
{
	g->end           = ms[len-1]; // TODO: make sure this is a valid assumption
	//g->end_v        = &vs[len-1];
	g->idmap         = idmap;
	g->idmap_len     = idmap_len;
	g->masks         = ms;
	g->vertex_count  = len;
	g->adjacency     = adjacency;
	g->max_neighbors = max_neighbors;
	g->combo_len     = combo_len;
	g->counts        = calloc(len, sizeof(*g->counts));

	if (g->counts == NULL)
	{
		printf("init_graph: malloc failure\n");
		exit(-1);
	}
}

#endif
