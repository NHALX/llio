
#define F_ID                         0
#define F_PASSIVE                    1
#define F_COST                       2 
#define F_AD                         3
#define F_CRIT_CHANCE                4
#define F_CRIT_BONUS                 5
#define F_ATTACK_SPEED               6 
#define F_ARMORPEN_FLAT              7
#define F_ARMORPEN_PERCENT           8
#define F_HP                         9
#define F_HP2AD                      10
#define ITEM_WIDTH                   11
#define ITEM_PASSIVE_SYNC_OFFSET     F_AD /* Passive layout matches item layout after first few columns are dropped */
#define PASSIVE_WIDTH                (ITEM_WIDTH-ITEM_PASSIVE_SYNC_OFFSET)
#define PASSIVE_NULL                 0
