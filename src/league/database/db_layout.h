#ifndef _DB_LAYOUT_H_
#define _DB_LAYOUT_H_

#include "types.h"
#include "vector.h"
#include "db_info.h"

#define PASSIVE_NULL                 0
#define BUILDTREE_WIDTH              4
#define ITEM_T_SIZE                  256

typedef ushort_t itemid_t;

typedef struct {
	VECTOR(stats);                         
	VECTOR(passive);                        
	itemid_t id;                          // 2
	ushort_t   passive_id;                  // 4 
	ushort_t   total_cost;                  // 6
	ushort_t   upgrade_cost;                // 8
	ushort_t   slot_merge;                  // 10
    ushort_t   buildtree[BUILDTREE_WIDTH];  // ??
	unsigned char pad[ITEM_T_SIZE - ((VECTOR_SIZEOF*2) + 10 + (2 * BUILDTREE_WIDTH))];                   // pad to 64 bytes (power of 2)
#ifdef __OPENCL_VERSION__
} __attribute__ ((aligned (ITEM_T_SIZE))) item_t;
#else
} item_t; static_assert(sizeof (item_t) == ITEM_T_SIZE, "sizeof(item_t) != ITEM_T_SIZE");
#endif

#define F_AD(X)                         PART(X,0)
#define F_CRIT_CHANCE(X)                PART(X,1)
#define F_CRIT_BONUS(X)                 PART(X,2)
#define F_ATTACK_SPEED(X)               PART(X,3)
#define F_ARMORPEN_FLAT(X)              PART(X,4)
#define F_ARMORPEN_PERCENT(X)           PART(X,5)
#define F_HP(X)                         PART(X,6)
#define F_HP2AD(X)                      PART(X,7)
#define F_HPRegen(X)                    PART(X,8)
#define F_LIFESTEAL_PERCENT(X)          PART(X,9)
#define F_LIFESTEAL_FLAT(X)             PART(X,10)
#define F_SPELLVAMP(X)                  PART(X,11)

// TODO: rename this file db_stat or something
// TODO: handle stats that are merged multiplicatively 
#ifndef __OPENCL_VERSION__
#define __constant const
#endif

__inline void
stats_add(__constant item_t db_items[], VECTOR(*stats), size_t item_id, bool_t copy_passive)
{
    if (copy_passive)
        VECTOR_ADD(*stats, db_items[item_id].passive);

    VECTOR_ADD(*stats, db_items[item_id].stats);
}

__inline void
stats_remove(__constant item_t db_items[], VECTOR(*stats), size_t item_id, bool_t copy_passive)
{
    if (copy_passive)
        VECTOR_SUB(*stats, db_items[item_id].passive);

    VECTOR_SUB(*stats, db_items[item_id].stats);
}


#endif
