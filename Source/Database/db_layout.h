
#ifndef _DB_LAYOUT_H_
#define _DB_LAYOUT_H_


#ifndef __OPENCL_VERSION__

#ifdef __APPLE__
#include <OpenCL/cl.h>
#else
#include <CL/cl.h>
#endif
#define STATS_T_VEC_N 8
typedef cl_short8  stats_t;
typedef cl_ushort  c_ushort;
typedef cl_uint    c_uint;
typedef cl_float   c_float;
typedef cl_short   c_itemid_t;
    
#define F_AD(X)                         ((X).s[0])
#define F_CRIT_CHANCE(X)                ((X).s[1])
#define F_CRIT_BONUS(X)                 ((X).s[2])
#define F_ATTACK_SPEED(X)               ((X).s[3])
#define F_ARMORPEN_FLAT(X)              ((X).s[4])
#define F_ARMORPEN_PERCENT(X)           ((X).s[5])
#define F_HP(X)                         ((X).s[6])
#define F_HP2AD(X)                      ((X).s[7])

#else
#define STATS_T_VEC_N 8
typedef short8  stats_t;
typedef ushort  c_ushort;
typedef uint    c_uint;
typedef float   c_float;
typedef short   c_itemid_t;

#define F_AD(X)                         ((X).S0)
#define F_CRIT_CHANCE(X)                ((X).S1)
#define F_CRIT_BONUS(X)                 ((X).S2)
#define F_ATTACK_SPEED(X)               ((X).S3)
#define F_ARMORPEN_FLAT(X)              ((X).S4)
#define F_ARMORPEN_PERCENT(X)           ((X).S5)
#define F_HP(X)                         ((X).S6)
#define F_HP2AD(X)                      ((X).S7)

#endif

#define PASSIVE_NULL                 0
#define BUILDTREE_WIDTH              4

typedef struct {
    stats_t    stats;                       // 16
    stats_t    passive;                     // 32
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

#endif
