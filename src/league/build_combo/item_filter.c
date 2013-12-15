#include <assert.h>
#include <malloc.h>
#include <string.h>

#include "league/database/database.h"
#include "league/ll_formulas.h"
#include "league/common.h"
#include "league/build_combo/item_filter.h"
#include "combinatorics/combination.h"



static bool_t
Test(const item_t *item, uint_t flags)
{
    VECTOR(stats) = VECTOR_ZERO_INIT;
    VECTOR_ADD(stats, item->stats);
    VECTOR_ADD(stats, item->passive);

    if ((flags & FC_SUSTAIN) && (
        F_HPRegen(stats) ||
        F_LIFESTEAL_PERCENT(stats) ||
        F_LIFESTEAL_FLAT(stats) ||
        F_SPELLVAMP(stats)
        ))
        return TRUE;

    if ((flags & FC_PHYSICAL_DAMAGE) && (
        F_AD(stats) ||
        F_CRIT_CHANCE(stats) ||
        F_CRIT_BONUS(stats) ||
        F_ATTACK_SPEED(stats) ||
        F_ARMORPEN_FLAT(stats) ||
        F_ARMORPEN_PERCENT(stats) ||
        F_HP2AD(stats)
        ))
        return TRUE;

    if (flags & FC_MAGIC_DAMAGE)
    {
        assert(0);
    }

    if ((flags & FC_SURVIVABILITY) && (
        F_HP(stats)
        ))
        return TRUE;
 
    return FALSE;
}

item_t*
item_filter(uint_t flags, size_t *count)
{
    item_t *filtered;
    *count = 0;
    for (size_t i = 0; i < DB_LEN; ++i)
    {
        if (Test(&db_items[i], flags))
            (*count)++;
    }

    filtered = malloc((*count) * sizeof *filtered);
    assert(filtered);
    if (!filtered)
        return 0;

    for (size_t i = 0, j = 0; i < DB_LEN; ++i)
    {
        if (Test(&db_items[i], flags))
            memcpy(&filtered[j++], &db_items[i], sizeof *filtered);
    }

    return filtered;
}
