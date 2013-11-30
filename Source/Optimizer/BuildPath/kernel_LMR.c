#ifndef __OPENCL_VERSION__ 
#include "../Common/OpenCLDebugHack.h"
#include <limits.h>
#endif

#include "../../types.h"
#include "../../Database/db_layout.h"
#include "../Common/ll_formulas.h"
#include "opencl_kernel.h"

////////////////////////////////////////////////////////
#define PASV_SCRATCH_LEN(X) (X+1)

#define ZERO_INIT(BUF, N) do { \
for (size_t zi = 0; zi < N; ++zi)\
	BUF[zi] = 0; \
} while (0)



// TODO: handle stats that are merged multiplicatively 
inline void
MergeStats(DB, VECTOR(*stats), size_t item_id, bool copy_passive)
{
	if (copy_passive)
		VECTOR_ADD(*stats, db_items[item_id].passive);

	VECTOR_ADD(*stats, db_items[item_id].stats);
}

inline void
RemoveStats(DB, VECTOR(*stats), size_t item_id, bool copy_passive)
{
	if (copy_passive)
		VECTOR_SUB(*stats, db_items[item_id].passive);

	VECTOR_SUB(*stats, db_items[item_id].stats);
}


inline int
AddItem(DB, __local ideal_t *unique, VECTOR(*stats), size_t item)
{
	size_t passive_id;
	int removed_n = 0;

	if (item == 0)
		return 1;
	
	for (size_t i = 0; i < BUILDTREE_WIDTH; ++i)
	{
		size_t id = db_items[item].buildtree[i];

		if (id == 0)
			continue;

		passive_id = db_items[id].passive_id;
		RemoveStats(db_items, stats, id, --unique[passive_id] == 0);
		++removed_n;
	}

	passive_id = db_items[item].passive_id;
	MergeStats(db_items, stats, item, ++unique[passive_id] == 1);
	return 1 - removed_n;
}


// ASSUMES: item passive_id's arent larger than LINEXT_WIDTH_MAX and 
// that there is less than LINEXT_WIDTH_MAX passives being used 

static
int
MetricAreaDPS(DB,
	llf_criteria *cfg,
	itemid_t *dbi,
	uint dbi_n,
	__local ideal_t *unique,
	float *output)
{
	size_t bs;
	uint inventory_slots;
	float result   = 0;
	float prev_dps = 0;

	VECTOR(stats) = VECTOR_ZERO_INIT;


	for (bs = 0, inventory_slots = 0; bs < dbi_n; ++bs) 
	{	
		size_t item = dbi[bs];

		inventory_slots += AddItem(db_items, unique, (&stats), item);
		
		if (inventory_slots > cfg->build_maxinventory) // invalid build path
			{ *output = 0; return ERROR_INVENTORY; }

		result += (prev_dps * ((float)db_items[dbi[bs]].upgrade_cost));
		prev_dps = llf_dmgtotal(cfg, &stats); 
	}
	 
	*output = result;
	return ERROR_NONE;
}


//////////////////////////// Generate linear extension  ////////////////////////////

#define GET_EDGES(P,S,I) (&P[I*S])
#ifndef __OPENCL_VERSION__

void
k_linext_print(linext_t *le)
{
	unsigned int i;

	DEBUG_PRINTF("%c", '{');
	for (i = 0; i < le->le_len; ++i)
	{
		DEBUG_PRINTF("%d", le->le_buf[i]);

		if (i + 1 < le->le_len)
			DEBUG_PRINTF("%c", ',');
	}

	DEBUG_PRINTF("%c\n", '}');
}
#endif

ideal_t
k_linext_extend(ideal_t ideal, linext_t *le)
{
	if (ideal != 0 && le->le_index > 0)
	{
		le->le_index--;
		le->le_buf[le->le_index] = ideal;
	}
	return ideal;
}


int k_linext_nth(
	__global ideal_t *ideals,
	__global count_t *counts,
	__global index_t *adjacency,
	uint  max_neighbors,
	count_t nth_extension,
	linext_t *le)
{
	__global index_t *neighbors;
	__global ideal_t *iedges;
	size_t i;
	int node_index = 0; // TODO: make sure this is a valid assumption


	while (le->le_index > 0)
	{
		count_t interval[2];
		count_t prev_count;
		ideal_t ideal;
		
		ideal      = 0;
		neighbors  = GET_EDGES(adjacency, max_neighbors, node_index);
		iedges     = GET_EDGES(ideals, max_neighbors, node_index);
		node_index = -1;

		for (prev_count = 0, i = 0; i < max_neighbors; ++i)
		{
			if (neighbors[i] == INVALID_NEIGHBOR)
				continue; // TODO: break instead?

			node_index = neighbors[i];

			interval[0] = prev_count;
			prev_count += counts[node_index];
			interval[1] = prev_count - 1;

			if (nth_extension >= interval[0] && nth_extension <= interval[1])
			{
				ideal = iedges[i];
				break;
			}
		}

		if (ideal == 0) // error, nothing found
			return 1;

		k_linext_extend(ideal, le);
		nth_extension -= interval[0];
	}

	return 0;
}


// CPU version calls this function directly
result_t k_buildpath(
	DB,
	llf_criteria *cfg,
	__local ideal_t *pasv_scratch,

	__global ideal_t *ideals,
	__global count_t *counts,
	__global index_t *adjacency,
	buildpath_info *info,

	size_t global_id,
    size_t local_id
	)
{
	count_t nth_extension;
	result_t metric;
	
	nth_extension = info->linext_offset + global_id;
    pasv_scratch += PASV_SCRATCH_LEN(info->linext_width) * local_id;

	ZERO_INIT(pasv_scratch, PASV_SCRATCH_LEN(info->linext_width));

	metric.metric = 0;
	metric.index  = ERROR_IGNORED;

	if (nth_extension < info->linext_count)
	{
		linext_t le;
		itemid_t ids[LINEXT_WIDTH_MAX]; 
		size_t i;
		int error;

		le.le_index = info->linext_width;
		le.le_len = info->linext_width;

		k_linext_nth(ideals, counts, adjacency, info->max_neighbors, nth_extension, &le);

		// NOTE: We dont actually need the extra precision here, but if we get rid of it
		// some of the support functions won't be able to index the full database.
		for (i = 0; i < info->linext_width; i++)
			ids[i] = le.le_buf[i]; 
		
		error = MetricAreaDPS(db_items, cfg, ids, info->linext_width, pasv_scratch, &metric.metric);	
		metric.index = (error != ERROR_NONE) ? error : nth_extension;
	}

	return metric;
}

