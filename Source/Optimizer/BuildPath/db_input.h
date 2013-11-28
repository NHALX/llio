#ifndef _DB_INPUT_H_
#define _DB_INPUT_H_
#include "x_types.h"
#include "../../Database/db_layout.h"

#define BUILDTREE_MAX 9 // TODO: pull this from db
#define IDMAP_MAX_WIDTH (BUILDTREE_MAX*2)

c_ideal_t dbi_poset(char **items, size_t item_len, c_itemid_t *node2dbi, c_ideal_t poset[][2], size_t *p_len);
item_t*dbi_filter(size_t vertex_n, c_itemid_t *node2dbi);
size_t dbi_find(c_itemid_t item);

#endif
