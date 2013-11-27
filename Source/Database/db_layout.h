#ifndef _DB_LAYOUT_H_
#define _DB_LAYOUT_H_

#include "vector.h"
#include "db_info.h"

#define PASSIVE_NULL                 0
#define BUILDTREE_WIDTH              4

typedef struct {
	VECTOR(stats);                          // 16
	VECTOR(passive);                        // 32
	c_itemid_t id;                          // 34
	c_ushort   passive_id;                  // 36 
	c_ushort   total_cost;                  // 38
	c_ushort   upgrade_cost;                // 40
	c_ushort   slot_merge;                  // 42
    c_ushort   buildtree[BUILDTREE_WIDTH];  // ??
    unsigned char pad[64-(42+(2*BUILDTREE_WIDTH))];                   // pad to 64 bytes (power of 2)
#ifdef __OPENCL_VERSION__
} __attribute__ ((aligned (64))) item_t;
#else
} item_t; static_assert(sizeof (item_t) == 64, "sizeof(item_t) != 64");
#endif

#define F_AD(X)                         PART(X,0)
#define F_CRIT_CHANCE(X)                PART(X,1)
#define F_CRIT_BONUS(X)                 PART(X,2)
#define F_ATTACK_SPEED(X)               PART(X,3)
#define F_ARMORPEN_FLAT(X)              PART(X,4)
#define F_ARMORPEN_PERCENT(X)           PART(X,5)
#define F_HP(X)                         PART(X,6)
#define F_HP2AD(X)                      PART(X,7)

#endif
