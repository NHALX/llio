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


static int ItemCmp(const void * a, const void * b)
{
	return (int)((float*)a)[F_ID] - (int)((float*)b)[F_ID];
}

// TODO: verify db is actually sorted somewhere.
static size_t FindItemIndex(c_itemid_t item) // ASSUMES: database is sorted
{
	float key[ITEM_WIDTH];
	uintptr_t entry;

	key[F_ID] = item;
	entry = (uintptr_t)bsearch(&key, db_items, DB_LEN, ITEM_SIZEOF, &ItemCmp);

	assert(entry);
	return (!entry) ? -1 : (entry - (uintptr_t)db_items) / ITEM_SIZEOF;
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


/////////////////////////// ConvertPoset /////////////////////////////

static int icmp(const void * a, const void * b)
{
	return (**(cl_short**)a) - (**(cl_short**)b);
}


static
size_t PFind(c_ideal_t poset[][2], size_t n, size_t index)
{
	size_t i;
	for (i = 0; i < n; ++i){
		if (poset[i][0] == index + 1)
			return i;
	}
	assert(0);
	return -1;
}


c_ideal_t ConvertPoset(cl_short input[][2], size_t n, c_itemid_t *idmap, c_ideal_t poset[][2], size_t *poset_n)
{
	size_t i, j, prev;
	cl_short **set = alloca(sizeof(*set)* n * 2);
	cl_short(*local)[2] = alloca(sizeof(cl_short)* n * 2); // TODO: change size calc to use *local
	c_ideal_t idmi = 0;
	c_ideal_t *matrix;
	size_t dim;

	memcpy(local, input, sizeof(cl_short)* n * 2);

	assert(n <= C_IDEAL_T_MAX);

	if (n > C_IDEAL_T_MAX)
		n = C_IDEAL_T_MAX;

	// sort by item id
	for (i = 0, j = 0; i < n; ++i)
	{
		set[j++] = &local[i][0];
		set[j++] = &local[i][1];
	}

	qsort(set, n * 2, sizeof *set, &icmp);

	// map item ids to vertex #s
	for (i = 0, prev = 0; i < n * 2; ++i)
	{
		if (prev == 0 || *set[i] != prev)
		{
			idmap[idmi] = *set[i];//FindItemIndex(*set[i]);
			prev = *set[i];
			++idmi;
		}

		*set[i] = idmi;
	}

	// copy vertex #s to output
	for (i = 0; i < n; ++i){
		poset[i][0] = (c_ideal_t)local[i][0];
		poset[i][1] = (c_ideal_t)local[i][1];
	}

	dim = idmi;
	matrix = alloca(sizeof(*matrix) * dim * dim);
	memset(matrix, 0, sizeof(*matrix) * dim * dim);

	// create graph adjacency matrix
	for (i = 0; i < n; ++i)
		++matrix[INDEX2(dim, poset[i][0] - 1, poset[i][1] - 1)];


	// make duplicates unique
	for (i = 0; i < dim; ++i) // rows
	{
		size_t out_degree;
		c_itemid_t item_id;

		// count vertex out degree in the graph
		for (j = 0, out_degree = 0; j < dim; ++j)
			out_degree += matrix[INDEX2(dim, i, j)];

		if (out_degree == 0)
			continue;

		item_id = input[PFind(poset, n, i)][0]; // FindItemIndex(input[PFind(poset, n, i)][0]);

		// resolve duplicates
		
		for (j = 0; out_degree > 1 && j < dim; ++j)
		{
			if (matrix[INDEX2(dim, i, j)] > 0)
			{
				--matrix[INDEX2(dim, i, j)];
				--out_degree;

				idmap[idmi] = item_id;
				poset[PFind(poset, n, i)][0] = ++idmi;
			}
		}
		
	}

	*poset_n = n;
	return idmi;
}


/////////////////////////// dbi_poset /////////////////////////////
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
		printf("%s(%d)\n", db_names[node2dbi[i]], i);
}

void
EdgeCopy(size_t index, cl_short **ptr, size_t *len)
{
	size_t j;

	for (j = 0; j < BUILDTREE_WIDTH && db_buildtree[index][j] != 0; ++j)
	{
		c_itemid_t child = db_buildtree[index][j];
		(*ptr)[0] = child;
		(*ptr)[1] = index;
		(*len)++;
		(*ptr) += 2;
		EdgeCopy(child, ptr, len);
	}
}


c_ideal_t
dbi_poset(char **items, size_t item_len, c_itemid_t *node2dbi, c_ideal_t poset[][2], size_t *p_len)
{
	size_t i;
	cl_short (*edges)[2] = alloca(BUILDTREE_MAX*item_len*sizeof *edges);
	cl_short *ptr = (void*)edges;
	c_ideal_t vertex_count;
	cl_short *unconnected = alloca(item_len*sizeof *edges);
	size_t unconnected_n = 0;

	for (*p_len = 0, i = 0; i < item_len; ++i)
	{
		size_t index = FindItemIndexS(items[i]);

		if (db_buildtree[index][0] == 0)
			unconnected[unconnected_n++] = index;
		else
			EdgeCopy(index, &ptr, p_len);
	}

	vertex_count = ConvertPoset(edges, *p_len, node2dbi, poset, p_len);
	memcpy(&node2dbi[vertex_count], unconnected, unconnected_n*sizeof *unconnected);
	vertex_count += unconnected_n;
	PrintPoset(node2dbi, vertex_count, poset, *p_len); // TODO: move this to a unit test
	return vertex_count;
}


