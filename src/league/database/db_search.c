#include <stdlib.h>
#include <assert.h>
#include "league/database/db_search.h"
#include "league/database/database.h"

static int item_cmp_id(const void * a, const void * b)
{
    return ((item_t*)a)->id - ((item_t*)b)->id;
}

#define PTR2INDEX(E, B, SIZE) \
    (!E) ? -1 : ((uintptr_t)E - (uintptr_t)B) / SIZE;


// TODO: verify db is actually sorted somewhere.
size_t db_find(itemid_t item) // ASSUMES: database is sorted
{
    item_t key;
    uintptr_t entry;

    key.id = item;
    entry = (uintptr_t)bsearch(&key, db_items, DB_LEN, sizeof key, &item_cmp_id);

    assert(entry);
    return PTR2INDEX(entry, db_items, sizeof key);
}
