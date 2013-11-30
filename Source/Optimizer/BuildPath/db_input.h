#ifndef _DB_INPUT_H_
#define _DB_INPUT_H_
#include "../../types.h"
#include "lattice.h"
#include "../../Database/db_layout.h"

#define BUILDTREE_MAX 9 // TODO: pull this from db
#define IDMAP_MAX_WIDTH (BUILDTREE_MAX*2)

ideal_t dbi_poset(char **items, size_t item_len, itemid_t *node2dbi, ideal_t poset[][2], size_t *p_len);
item_t*dbi_filter(size_t vertex_n, itemid_t *node2dbi, size_t *db_len);
size_t dbi_find(itemid_t item);

#endif
