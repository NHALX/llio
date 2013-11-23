#ifndef _DB_INPUT_H_
#define _DB_INPUT_H_
#include "x_types.h"

#define BUILDTREE_MAX 9 // TODO: pull this from db
#define IDMAP_MAX_WIDTH (BUILDTREE_MAX*2)

c_ideal_t dbi_poset(char **items, size_t item_len, c_itemid_t *node2dbi, c_ideal_t poset[][2], size_t *p_len);

#endif
