
#ifndef __OPENCL_VERSION__
#ifdef __OSX__
#include <OpenCL/cl.h>
#else
#include <CL/cl.h>
#endif
#endif

#define F_ID                         0
#define F_PASSIVE                    1
#define F_COST                       2 
#define F_UPGRADE_COST               3
#define F_SLOT_MERGE                 4
#define F_AD                         5
#define F_CRIT_CHANCE                6
#define F_CRIT_BONUS                 7
#define F_ATTACK_SPEED               8 
#define F_ARMORPEN_FLAT              9
#define F_ARMORPEN_PERCENT           10
#define F_HP                         11
#define F_HP2AD                      12
#define ITEM_WIDTH                   13
#define ITEM_PASSIVE_SYNC_OFFSET     F_AD /* Passive layout matches item layout after first few columns are dropped */
#define ITEM_SIZEOF                  (ITEM_WIDTH*sizeof(cl_float))
#define PASSIVE_WIDTH                (ITEM_WIDTH-ITEM_PASSIVE_SYNC_OFFSET)
#define PASSIVE_NULL                 0
#define BUILDTREE_WIDTH              4

#define DB_LEN 218
extern const cl_float db_items[DB_LEN][ITEM_WIDTH];
extern const cl_float db_passives[DB_LEN][ITEM_WIDTH-ITEM_PASSIVE_SYNC_OFFSET];
extern const char *db_names[DB_LEN];
extern const cl_short db_buildtree[DB_LEN][BUILDTREE_WIDTH];
