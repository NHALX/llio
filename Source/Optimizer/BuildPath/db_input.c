#ifndef _MSC_VER
#include <alloca.h>
#else
#define alloca _alloca
#endif
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include "db_input.h"
#include "../../Database/database.h"


static int item_cmp_id(const void * a, const void * b)
{
	return ((item_t*)a)->id - ((item_t*)b)->id;
}

#define PTR2INDEX(E, B, SIZE) \
	(!E) ? -1 : ((uintptr_t)E - (uintptr_t)B) / SIZE;


// TODO: verify db is actually sorted somewhere.
size_t dbi_find(c_itemid_t item) // ASSUMES: database is sorted
{
	item_t key;
	uintptr_t entry;

	key.id = item;
	entry = (uintptr_t)bsearch(&key, db_items, DB_LEN, sizeof key, &item_cmp_id);

	assert(entry);
	return PTR2INDEX(entry, db_items, sizeof key);
}


static size_t FindItemIndexS(char *item) // TODO: make less terrible.
{
	size_t i;

	for (i = 0; i < sizeof db_items / sizeof *db_items; i++)
	{
		if (strcmp(db_names[i], item) == 0)
			return i;
	}

	assert(0);
	return (size_t)-1;
}


#define DEPENDENCY_DEBUG

typedef struct tree_t 
{
	size_t v_index;
	struct tree_t *requires[BUILDTREE_WIDTH];

	#ifdef DEPENDENCY_DEBUG
	size_t db_index;
	const char * name;
	#endif
	
} tree_t;


tree_t *
Tree(size_t db_index, c_itemid_t *node2dbi, size_t *v_index)
{
	size_t j;
	tree_t *t = calloc(1, sizeof *t);
	
	assert(t);
	if (!t)
		return 0;
	
	++(*v_index);

	t->db_index = db_index;
	#ifdef DEPENDENCY_DEBUG
	t->v_index  = *v_index;
	t->name     = db_names[db_index];
	#endif

	node2dbi[(*v_index)-1] = db_index;
	
	for (j = 0; j < BUILDTREE_WIDTH && db_items[db_index].buildtree[j] != 0; ++j)
		t->requires[j] = Tree(db_items[db_index].buildtree[j], node2dbi, v_index);

	return t;
}


void
TreeFree(tree_t *t)
{
	size_t j;
	for (j = 0; j < BUILDTREE_WIDTH && t->requires[j] != 0; ++j)
		TreeFree(t->requires[j]);

	free(t);
}


void
EdgeCopy(tree_t *t, c_ideal_t **ptr, size_t *len)
{
	size_t j;

	for (j = 0; j < BUILDTREE_WIDTH && t->requires[j] != 0; ++j)
	{
		tree_t *child = t->requires[j];
		(*ptr)[0] = child->v_index;
		(*ptr)[1] = t->v_index;
		(*len)++;
		(*ptr) += 2;
		EdgeCopy(child, ptr, len);
	}
}

void
PrintPoset(c_itemid_t *node2dbi, size_t n2d_n, c_ideal_t poset[][2], size_t poset_n)
{
	size_t i;
	printf("======= poset: =======\n");

	for (i = 0; i < poset_n; ++i)
	{
		printf("%s(%d) -> %s(%d)\n",
			db_names[node2dbi[poset[i][0] - 1]], poset[i][0],
			db_names[node2dbi[poset[i][1] - 1]], poset[i][1]);
	}

	printf("======= mappings: =======\n");

	for (i = 0; i < n2d_n; ++i)
		printf("%s(%d)\n", db_names[node2dbi[i]], i+1);
}


c_ideal_t
dbi_poset(char **items, size_t item_len, c_itemid_t *node2dbi, c_ideal_t poset[][2], size_t *p_len)
{
	size_t i;
	c_ideal_t *ptr = (void*)poset;
	size_t vertex_count = 0;

	for (*p_len = 0, i = 0; i < item_len; ++i)
	{
		tree_t *t = Tree(FindItemIndexS(items[i]), node2dbi, &vertex_count);
		EdgeCopy(t, &ptr, p_len);
		TreeFree(t);
	}

	PrintPoset(node2dbi, vertex_count, poset, *p_len); // TODO: move this to a unit test
	return vertex_count;
}


item_t*
dbi_filter(size_t vertex_n, c_itemid_t *node2dbi)
{
	item_t* db_filtered = calloc(++vertex_n, sizeof *db_filtered);

	for (size_t i = 1; i < vertex_n; ++i)
		memcpy(&db_filtered[i], &db_items[node2dbi[i-1]], sizeof *db_filtered);
	
	for (size_t i = 0; i < vertex_n; ++i)
	{
		for (size_t j = 0; j < BUILDTREE_WIDTH; ++j)
		{
			size_t component = db_filtered[i].buildtree[j];

			if (!component)
				continue;

			for (size_t k = 0; k < vertex_n; ++k)
			{
				if (db_filtered[k].id == db_items[component].id)
					db_filtered[i].buildtree[j] = k;
			}
		}
	}

	// TODO: remap passive_ids and be certain they do not exceed LINEXT_MAX_WIDTH
	return db_filtered;
}
