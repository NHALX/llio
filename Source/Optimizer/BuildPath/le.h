#ifndef _LE_H_
#define _LE_H_


//#pragma pack (push, 16)
typedef struct { cl_float metric; cl_ulong index; }    c_result_t;
//#pragma pack(pop)

#define IDEAL_T_MAX CL_UCHAR_MAX
typedef cl_uint  c_index_t;
typedef cl_uchar c_ideal_t;
typedef cl_ulong c_count_t;
typedef cl_short c_itemid_t;

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


static __inline void init_graph(
	struct graph *g, 
	unsigned int len,
	c_ideal_t *ideals,
	c_index_t *adjacency,
	unsigned int max_neighbors, 
	unsigned int combo_len,
	c_itemid_t *idmap,
	unsigned int idmap_len)
{
	g->end           = len-1; // TODO: make sure this is a valid assumption
	g->idmap         = idmap;
	g->idmap_len     = idmap_len;
	g->ideals        = ideals;
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
