#ifndef _LE_H_
#define _LE_H_

#include "x_types.h"

//#pragma pack (push, 16)
typedef struct { cl_float metric; cl_ulong index; }    c_result_t;
//#pragma pack(pop)



struct graph {
	c_index_t  end;
	c_count_t  *counts;
	c_itemid_t *idmap;
	c_index_t  *adjacency;
	c_ideal_t  *ideals;
	unsigned int idmap_len;
	unsigned int vertex_count;
	unsigned int max_neighbors;
	unsigned int combo_len;
};


// TODO: this function is out of place
//       also, have callers check return value since it used to be void.

static __inline int init_graph(
	struct graph *g, 
	unsigned int len,
	c_ideal_t *ideals,
	c_index_t *adjacency,
	unsigned int max_neighbors, 
	unsigned int combo_len,
	c_itemid_t *idmap,
	unsigned int idmap_len)
{
	c_count_t *counts;
	
	GUARD(counts = calloc(len, sizeof(*g->counts)));

	g->end           = len-1; // TODO: make sure this is a valid assumption
	g->idmap         = idmap;
	g->idmap_len     = idmap_len;
	g->ideals        = ideals;
	g->vertex_count  = len;
	g->adjacency     = adjacency;
	g->max_neighbors = max_neighbors;
	g->combo_len     = combo_len;
	g->counts        = counts;
}

#endif
