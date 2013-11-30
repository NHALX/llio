#include "../db_input.h"
#include "../../../Database/database.h"
#include <assert.h>


void unittest_db_input()
{
	item_t *items;
	#define FILTER_N 6
	itemid_t filter[FILTER_N] = { 14, 14, 130, 20, 102, 137 }; // Youmou's ghostblade
	size_t db_len;

	items = dbi_filter(FILTER_N, filter, &db_len);
	assert(items);

	assert(0 == items[0].id);
	assert(db_items[14].id  == items[1].id);
	assert(db_items[14].id  == items[2].id);
	assert(db_items[130].id == items[3].id);
	assert(db_items[20].id  == items[4].id);
	assert(db_items[102].id == items[5].id);
	assert(db_items[137].id == items[6].id);

	assert(db_items[130].buildtree[0] == 14);
	assert(db_items[130].buildtree[1] == 14);

	assert(items[3].buildtree[0] == 1 || items[3].buildtree[0] == 2);
	assert(items[3].buildtree[1] == 1 || items[3].buildtree[0] == 2);

	assert(db_items[102].buildtree[0] == 20);
	assert(items[5].buildtree[0] == 4);

	assert(db_items[137].buildtree[0] == 130 || db_items[137].buildtree[0] == 102);
	assert(db_items[137].buildtree[1] == 130 || db_items[137].buildtree[1] == 102);

	assert(items[6].buildtree[0] == 5 || items[6].buildtree[0] == 3);
	assert(items[6].buildtree[1] == 5 || items[6].buildtree[1] == 3);

}