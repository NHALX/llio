#ifndef __OPENCL_VERSION__ 
#include "../Common/OpenCLDebugHack.h"
#include <limits.h>
#endif

#include "le_kernel.h"

#include "../Common/ll_formulas.h"
#include "../../Database/db_layout.h"


////////////////////////////////////////////////////////

// TODO: simplify this
static
float 
MetricAreaDPS(
	__constant float items[][ITEM_WIDTH],
	__constant float passives[][PASSIVE_WIDTH],
	__constant short buildtree[][BUILDTREE_WIDTH],
	__constant float *cfg,
	itemid_t *le_dbi,
	uint combo_len)
{

	size_t build_step;

	float result  = 0;
	float prev_dps = 0;
	int inventory_slots;
	

	for (build_step = 0; build_step < combo_len; ++build_step) // TODO: This can be optimized
	{
		__constant float *passive_unique[LINEXT_WIDTH_MAX]; // must be less than or eq combo_len
		__constant short *subcomponents = buildtree[le_dbi[build_step]];
		size_t passive_unique_len;
		float stats[ITEM_WIDTH];
		size_t idi, i;
		size_t subcomponent_len = (uint) items[le_dbi[build_step]][F_SLOT_MERGE];

		for (i = 0; i < ITEM_WIDTH; ++i) // initialize
			stats[i] = 0;

		inventory_slots    = 0;
		passive_unique_len = 0;

		// remove the item's sub-components from the set
		
		for (i = 0; i < subcomponent_len; ++i)
		{
			size_t k;
			size_t child = subcomponents[i];

			for (k = 0; k < combo_len; ++k)
				if (le_dbi[k] == child)
					le_dbi[k] = 0; // NOTE: This requires the 0th db entry to be a null item
		}


		for (idi = 0; idi < build_step + 1; ++idi)
		{
			__constant float *item;
			__constant float *passive;

			if (le_dbi[idi] == 0)
				continue;

			item             = items[le_dbi[idi]];
			passive          = passives[(uint)item[F_PASSIVE]];
			inventory_slots += 1;// - (uint)item[F_SLOT_MERGE]; nevermidn this is taken care of above

			if (inventory_slots > 6) // invalid build path
				return 0;

			for (i = 0; i < passive_unique_len; ++i) // see if passive is already in use
			{
				if (passive_unique[i] == passive)
					break;
			}

			if (i == passive_unique_len)
			{
				passive_unique[passive_unique_len++] = passive;

				for (i = 0; i < PASSIVE_WIDTH; ++i)
					stats[i + ITEM_PASSIVE_SYNC_OFFSET] += passive[i]; // TODO: handle stats that are merged multiplicatively
			}

			// TODO: store items as array of float_4s?
			// OPTIMIZATION: most time is spent in this loop and it has a lot of L2 cache misses 
			for (i = 0; i < ITEM_WIDTH; ++i) // merge item
				stats[i] += item[i]; // TODO: handle stats that are merged multiplicatively
		}

		result += (prev_dps * items[le_dbi[build_step]][F_UPGRADE_COST]);
		prev_dps = FORMULA_TOTAL(cfg, stats);
	}
		/*
		DEBUG_PRINTF("dps=%f\n", FORMULA_TOTAL(cfg, merged));
		DEBUG_PRINTF("cost=%f, metric=%f\n", items[id][F_UPGRADE_COST], metric / 1000);
	}

	DEBUG_PRINTF("merged={%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f}\n",
		merged[0], merged[1], merged[2], merged[3],
		merged[4], merged[5], merged[6], merged[7],
		merged[8], merged[9], merged[10], merged[11]);
	*/
	return result;
}

#ifdef __OPENCL_VERSION__
// Find maximum, NOTE: this requires a power of 2 local size
static
void 
Reduce(result_t dps, __local volatile result_t* scratch, __global result_t* result)
{
	int offset;
	int local_index;

	local_index = get_local_id(0);
	scratch[local_index] = dps;
	barrier(CLK_LOCAL_MEM_FENCE);

	// parallel reduction
	for (offset = get_local_size(0) / 2;
		offset > 0;
		offset >>= 1) //offset = offset / 2) 
	{
		if (local_index < offset) {
			result_t other = scratch[local_index + offset];
			scratch[local_index] = (dps.metric > other.metric) ? dps : other;
		}
		barrier(CLK_LOCAL_MEM_FENCE);
	}

	if (local_index == 0) {
		result[get_group_id(0)] = scratch[0];
	}
}
#endif

//////////////////////////// Generate linear extension  ////////////////////////////

#define GET_EDGES(P,S,I) (&P[I*S])

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
	__constant float items[][ITEM_WIDTH],
	__constant float passives[][PASSIVE_WIDTH],
	__constant short buildtree[][BUILDTREE_WIDTH],
	__constant float *cfg,
	__constant itemid_t node2id[],

	__global ideal_t *ideals,
	__global count_t *counts,
	__global index_t *adjacency,
	struct buildpath_info info,
	size_t global_id)
{
	count_t nth_extension;
	result_t metric;

	nth_extension = info.linext_offset + global_id;

	if (nth_extension >= info.linext_count)
	{
		metric.metric = 0;
		metric.index = 0;
	}
	else
	{
		linext_t le;
		itemid_t ids[LINEXT_WIDTH_MAX];
		size_t i;

		le.le_index = info.linext_width;
		le.le_len = info.linext_width;

		k_linext_nth(ideals, counts, adjacency, info.max_neighbors, nth_extension, &le);

		for (i = 0; i < info.linext_width; i++)
			ids[i] = node2id[le.le_buf[i] - 1]; // convert to zero based index

		metric.metric = MetricAreaDPS(items, passives, buildtree, cfg, ids, info.linext_width);
		metric.index = nth_extension;
	}

	return metric;
}

//////////////////////////// Kernel  ////////////////////////////
#ifdef __OPENCL_VERSION__
__kernel void kernel_buildpath(
	__constant float items[][ITEM_WIDTH],
	__constant float passives[][PASSIVE_WIDTH],
	__constant short buildtree[][BUILDTREE_WIDTH],
	__constant float *cfg,
	__constant itemid_t node2id[],

	__global ideal_t *ideals,
	__global count_t *counts,
	__global index_t *adjacency,
	struct buildpath_info info,

	__local result_t* scratch,
	__global result_t* result)
{
	result_t metric = k_buildpath(items, passives, buildtree, cfg, node2id, ideals, counts, adjacency, info, get_global_id(0));
	Reduce(metric, scratch, result);
}
#endif







