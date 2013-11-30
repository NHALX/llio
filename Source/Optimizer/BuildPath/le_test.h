#include "../../../types.h"

#define LE_T_ADJACENCY_MAX   3
#define LE_T_EXTENSION_SIZE	6
#define LE_T_IDMAP_LEN       (sizeof(node2id)/sizeof(*node2id))
#define LE_T_DATA_LEN        (sizeof(ideals) / sizeof(*ideals))

itemid_t node2id[] = { 103, 138, 21, 131, 15, 15 };

ideal_t ideals[][LE_T_ADJACENCY_MAX] = { { 0, 0, 0 }, { 3, 5, 6 }, { 1, 5, 6 }, { 3, 6, 0 }, { 3, 5, 0 }, { 5, 6, 0 }, { 1,
6, 0 }, { 1, 5, 0 }, { 3, 4, 0 }, { 6, 0, 0 }, { 5, 0, 0 }, { 1, 4, 0 }, { 3,
0, 0 }, { 4, 0, 0 }, { 1, 0, 0 }, { 2, 0, 0 }, { 0, 0, 0 } };

index_t adjacency[][LE_T_ADJACENCY_MAX] = { { 0, 0, 0 }, { 2, 3, 4 }, { 5, 6, 7 }, { 6, 8, 0 }, { 7, 8,
0 }, { 9, 10, 0 }, { 9, 11, 0 }, { 10, 11, 0 }, { 11, 12, 0 }, { 13, 0,
0 }, { 13, 0, 0 }, { 13, 14, 0 }, { 14, 0, 0 }, { 15, 0, 0 }, { 15, 0,
0 }, { 16, 0, 0 }, { 0, 0, 0 } };

