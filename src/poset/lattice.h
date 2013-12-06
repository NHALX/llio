#ifndef _LATTICE_H_
#define _LATTICE_H_

#include "tree.h"
#include "lattice_types.h"



typedef struct
{
	ideal_t *ideals;
	index_t *neighbors;
	count_t *counts;
	index_t source;
	index_t sink;

	size_t vertex_count;
    size_t edge_count;
	ideal_t max_neighbors;
	ideal_t linext_width;
	count_t linext_count;    // number of linear extensions
	struct ctx ctx;
}  ideal_lattice;


void lattice_free(ideal_lattice *lattice);
int lattice_create(ideal_t p_relations[][2], size_t p_reln, size_t n, ideal_lattice *lattice);
void glbinit_lattice();

#endif
