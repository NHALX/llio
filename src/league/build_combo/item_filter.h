#ifndef _BUILD_COMBO_ITEM_FILTER_H_
#define _BUILD_COMBO_ITEM_FILTER_H_

#include "types.h"
#include "league/database/db_layout.h"

#define FC_SUSTAIN         0x00000001
#define FC_PHYSICAL_DAMAGE 0x00000010
#define FC_MAGIC_DAMAGE    0x00000100
#define FC_SURVIVABILITY   0x00001000

item_t*item_filter(uint_t flags, size_t *n);

#endif
