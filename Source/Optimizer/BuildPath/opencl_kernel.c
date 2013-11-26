#ifndef __OPENCL_VERSION__ 
#include "../Common/OpenCLDebugHack.h"
#include <limits.h>
#endif

#include "le_kernel.h"

#include "../../Database/db_layout.h"
#include "../Common/ll_formulas.h"

////////////////////////////////////////////////////////
#ifdef __OPENCL_VERSION__
#define VECTOR_ADD(A,B) (A) += (B)
#define VECTOR_SUB(A,B) (A) -= (B)
#else
// TODO: handle stats that are merged multiplicatively 
#define VECTOR_ADD(A,B) do { \
	for (size_t I = 0; I < (sizeof ((A).s) / sizeof *((A).s)); ++I) \
		{ (A).s[I] += (B).s[I]; } \
	} while (0) 

#define VECTOR_SUB(A,B) do { \
for (size_t I = 0; I < (sizeof ((A).s) / sizeof *((A).s)); ++I) \
		{ (A).s[I] -= (B).s[I]; } \
	} while (0)
#endif

#define __dbstorage __global


inline void
MergeStats(__dbstorage item_t items[], stats_t *stats, size_t item_id, bool copy_passive)
{
	if (copy_passive)
		VECTOR_ADD(*stats, items[item_id].passive);

	VECTOR_ADD(*stats, items[item_id].stats);
}

inline void
RemoveStats(__dbstorage item_t items[], stats_t *stats, size_t item_id, bool copy_passive)
{
	if (copy_passive)
		VECTOR_SUB(*stats, items[item_id].passive);

	VECTOR_SUB(*stats, items[item_id].stats);
}

inline void
PassiveUnique(
	__dbstorage item_t items[],
	itemid_t *dbi,
	uint dbi_n,
	ushort2 uniqpasv[],
	ideal_t uniqpasv_idx[])
{
	size_t bs, i;
	size_t uniqpasv_n = 1;
	int found;
	 

	uniqpasv[0] = (ushort2){ 0, 0 }; // NOTE: assumes passive index 0 has empty stats

	for (bs = 0; bs < dbi_n; ++bs)
	{
		uint passive_id = items[dbi[bs]].passive_id;

		for (i = 0, found = -1; i < uniqpasv_n; ++i)
		{
			if (uniqpasv[i].x == passive_id)
				{ found = i; break;	}
		}

		if (found == -1)
		{
			found = uniqpasv_n;
			uniqpasv[uniqpasv_n++] = (ushort2){ passive_id, 0 };
		}

		uniqpasv_idx[bs] = found;
	}
}


inline int
ClearSubComponents(
	__dbstorage item_t items[],
	itemid_t *dbi,
	ushort2 unique[],
	ideal_t unique_i[],
	stats_t *stats,
	size_t build_step)
{
	__dbstorage short *subcomponents;
	int removed_n = 0;

	for (subcomponents = items[dbi[build_step]].buildtree; *subcomponents != 0; ++subcomponents)
	{
		for (size_t k = 0; k < build_step; ++k)
		{
			// NOTE: This assumes the 0th db entry to be a null item
			if (dbi[k] != 0 && dbi[k] == *subcomponents)
			{
				--unique[unique_i[k]].y;
				RemoveStats(items, stats, dbi[k], unique[unique_i[k]].y == 0);
				dbi[k] = 0; 
				removed_n++;
				break;
			}
		}
	}

	return removed_n;
}


// TODO: unit test PassiveUnique and ClearSubComp
static
int
MetricAreaDPS(
	__dbstorage item_t items[],
	__constant llf_criteria *cfg,
	itemid_t *dbi,
	uint dbi_n,
	float *output)
{
	size_t bs;
	uint inventory_slots;
	ushort2 unique[LINEXT_WIDTH_MAX];
	ideal_t unique_i[LINEXT_WIDTH_MAX];

	float result   = 0;
	float prev_dps = 0;
	stats_t stats  = (stats_t){ 0, 0, 0, 0, 0, 0, 0, 0 };
	

	PassiveUnique(items, dbi, dbi_n, unique, unique_i);
	
	for (bs = 0, inventory_slots = 0; bs < dbi_n; ++bs) // TODO: This can be optimized
	{	
		inventory_slots += 1 - ClearSubComponents(items, dbi, unique, unique_i, &stats, bs);
		
		if (inventory_slots > cfg->build_maxinventory) // invalid build path
			{ *output = 0; return ERROR_INVENTORY; }
	
		unique[unique_i[bs]].y++; // reference count
		MergeStats(items, &stats, dbi[bs], unique[unique_i[bs]].y == 1);

		result += (prev_dps * (float) items[dbi[bs]].upgrade_cost);
		prev_dps = llf_dmgtotal(cfg, &stats); // FORMULA_TOTAL(cfg, stats); 
	}
	 
	*output = result;
	return ERROR_NONE;
}

/*
// TODO: simplify this
static
float
MetricAreaDPS2(
__dbstorage item_t items[],
__dbstorage stats_t passives[],
__dbstorage short buildtree[][BUILDTREE_WIDTH],
__constant float *cfg,
itemid_t *le_dbi,
uint combo_len)
{

	size_t build_step;

	float result = 0;
	float prev_dps = 0;
	int inventory_slots;

	uint passive_unique[LINEXT_WIDTH_MAX]; // must be less than or eq combo_len
	size_t passive_unique_len;
	__dbstorage short *subcomponents;

	for (build_step = 0; build_step < combo_len; ++build_step) // TODO: This can be optimized
	{
		stats_t stats = (stats_t){ 0, 0, 0, 0, 0, 0, 0, 0 };
		size_t idi, i;

		inventory_slots = 0;
		passive_unique_len = 0;

		// remove the item's sub-components from the set
		for (subcomponents = buildtree[le_dbi[build_step]]; *subcomponents != 0; ++subcomponents)
		{
			size_t k;

			for (k = 0; k < combo_len; ++k)
			if (le_dbi[k] == *subcomponents)
				le_dbi[k] = 0; // NOTE: This requires the 0th db entry to be a null item
		}

		for (idi = 0; idi < build_step + 1; ++idi)
		{
			size_t item_id = le_dbi[idi];
			size_t passive_id;

			if (item_id == 0)
				continue;

			inventory_slots += 1;

			if (inventory_slots > 6) // invalid build path
				return 0;

			passive_id = items[item_id].passive;

			for (i = 0; i < passive_unique_len; ++i) // see if passive is already in use
			{
				if (passive_unique[i] == passive_id)
					break;
			}

			if (i == passive_unique_len)
			{
				passive_unique[passive_unique_len++] = passive_id;
				VECTOR_ADD(i, stats, passives[passive_id]);
			}

			// OPTIMIZATION: most time is spent in this loop and it has a lot of L2 cache misses 
			VECTOR_ADD(i, stats, items[item_id].stats);
		}

		result += (prev_dps * items[le_dbi[build_step]].upgrade_cost);
		prev_dps = FORMULA_TOTAL(cfg, stats);
	}
	*
	DEBUG_PRINTF("dps=%f\n", FORMULA_TOTAL(cfg, merged));
	DEBUG_PRINTF("cost=%f, metric=%f\n", items[id][F_UPGRADE_COST], metric / 1000);
	}

	DEBUG_PRINTF("merged={%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f}\n",
	merged[0], merged[1], merged[2], merged[3],
	merged[4], merged[5], merged[6], merged[7],
	merged[8], merged[9], merged[10], merged[11]);
	*
	return result;
}
*/

#ifdef __OPENCL_VERSION__
// Find maximum, NOTE: this requires a power of 2 local size
static
void 
Reduce(result_t dps, __local volatile result_t* scratch, __global result_t* result)
{
	int offset;
	int local_index;
	if (local_index == 0) {
		result[get_group_id(0)] = (result_t){ -12345, 12345 };
	}

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
	__dbstorage item_t items[],
	__constant llf_criteria *cfg,
	__constant itemid_t node2id[],

	__global ideal_t *ideals,
	__global count_t *counts,
	__global index_t *adjacency,
	struct buildpath_info info,
	size_t global_id)
{
	count_t nth_extension;
	result_t metric;
	// TODO: cahnge this
	//nth_extension = info.linext_offset + global_id;
	nth_extension = 1;
	metric.metric = 0;
	metric.index  = ERROR_IGNORED;

	if (nth_extension < info.linext_count)
	{
		linext_t le;
		itemid_t ids[LINEXT_WIDTH_MAX]; 
		size_t i;
		int error;

		le.le_index = info.linext_width;
		le.le_len = info.linext_width;

		k_linext_nth(ideals, counts, adjacency, info.max_neighbors, nth_extension, &le);

		for (i = 0; i < info.linext_width; i++)
			ids[i] = node2id[le.le_buf[i] - 1]; // convert to zero based index

		
		error = MetricAreaDPS(items, cfg, ids, info.linext_width, &metric.metric);	
		metric.index = (error != ERROR_NONE) ? error : nth_extension;
	}

	return metric;
}



#ifdef __OPENCL_VERSION__
	//////////////////////////// Kernel  ////////////////////////////

__kernel void kernel_buildpath(
	__dbstorage item_t items[],
	__constant llf_criteria *cfg,
	__constant itemid_t node2id[],

	__global ideal_t *ideals,
	__global count_t *counts,
	__global index_t *adjacency,
	struct buildpath_info info,

	__local result_t* scratch,
	__global result_t* result)
{
	result_t metric = k_buildpath(items, cfg, node2id, ideals, counts, adjacency, info, get_global_id(0));
	Reduce(metric, scratch, result);
}

//////////////////////// tests /////////////////////////

__kernel void kunittest_mem(
	__dbstorage item_t items[],
	__global itemid_t id[],
	__global ushort  stat_s0[],
	__global ushort  stat_s3[],
	__global ushort  stat_s6[],
	__global stats_t stat_all[])
{
	size_t i    = get_global_id(0);
	id[i]       = items[i].id;
	stat_s0[i]  = items[i].stats.s0;
	stat_s3[i]  = items[i].stats.s3;
	stat_s6[i]  = items[i].stats.s6;
	stat_all[i] = items[i].stats;
}


__kernel void kunittest_le(
	__global ideal_t *ideals,
	__global count_t *counts,
	__global index_t *adjacency,
	struct buildpath_info info,

	__global ideal_t* result)
{
	linext_t le;
	size_t i;
	size_t nth = get_global_id(0);

	le.le_index = info.linext_width;
	le.le_len = info.linext_width;

	k_linext_nth(ideals, counts, adjacency, info.max_neighbors, nth, &le);

	result += nth * info.linext_width;

	for (i = 0; i < info.linext_width; ++i)
		result[i] = le.le_buf[i];
}


__kernel void kunittest_mergestats(
	__dbstorage item_t items[],
	__global itemid_t *xs,
	uint xs_n,
	__global stats_t *output)
{
	stats_t stats = (stats_t){ 0,0,0,0,0,0,0,0 };
	size_t i;
	size_t id = get_global_id(0);

	xs += id * xs_n;

	for (i = 0; i < xs_n; ++i)
		MergeStats(items, &stats, xs[i], true);
	
	output[get_global_id(0)] = stats;
}


__kernel void kunittest_passiveuniqe(
	__dbstorage item_t items[],
	__global itemid_t *xs_in,
	uint xs_n,
	__global ushort2(*output)[LINEXT_WIDTH_MAX],
	__global ideal_t(*output_idx)[LINEXT_WIDTH_MAX]
	)
{
	ushort2 unique[LINEXT_WIDTH_MAX];
	ideal_t unique_i[LINEXT_WIDTH_MAX];
	itemid_t xs[LINEXT_WIDTH_MAX];

	size_t i;
	size_t id = get_global_id(0);

	xs_in += id * xs_n;
	for (i = 0; i < xs_n; ++i)
		xs[i] = xs_in[i];

	PassiveUnique(items, xs, xs_n, unique, unique_i);

	for (i = 0; i < xs_n; ++i)
	{
		output[id][i] = unique[i];
		output_idx[id][i] = unique_i[i];
	}

	for (i = xs_n; i < LINEXT_WIDTH_MAX; ++i)
	{
		output[id][i] = (ushort2){0,0};
		output_idx[id][i] = 0;
	}
}



__kernel void kunittest_clearsubcomponents(
	__dbstorage item_t items[],
	__global itemid_t *xs_in,
	uint xs_n,
	__global int(*output_inventory)[LINEXT_WIDTH_MAX],
	__global stats_t *output_stats)
{
	size_t x;
	int inventory_slots;
	ushort2 unique[LINEXT_WIDTH_MAX];
	ideal_t unique_i[LINEXT_WIDTH_MAX];
	itemid_t xs[LINEXT_WIDTH_MAX];
	stats_t stats = (stats_t){ 0, 0, 0, 0, 0, 0, 0, 0 };
	size_t id = get_global_id(0);

	xs_in += id * xs_n;
	for (x = 0; x < xs_n; ++x)
		xs[x] = xs_in[x];

	PassiveUnique(items, xs, xs_n, unique, unique_i);

	for (x = 0, inventory_slots = 0; x < xs_n; ++x)
	{
		inventory_slots += 1 - ClearSubComponents(items, xs, unique, unique_i, &stats, x);
		unique[unique_i[x]].y++; // reference count
		MergeStats(items, &stats, xs[x], unique[unique_i[x]].y == 1);

		output_inventory[id][x] = inventory_slots;
	}

	for (x = xs_n; x < LINEXT_WIDTH_MAX; ++x)
		output_inventory[id][x] = 0;

	output_stats[id] = stats;
}


__kernel void kunittest_llformulas(
	__dbstorage item_t items[],
	__global itemid_t *xs,
	__constant llf_criteria *cfg,
	__global float(*output)[7])
{
	size_t id = get_global_id(0);
	stats_t stats;
	float mit, ad, crit, aspd;

	stats = items[xs[id]].stats;
	mit = llf_armor_mitigation(cfg->enemy_armor, F_ARMORPEN_PERCENT(stats), F_ARMORPEN_FLAT(stats));
	ad = llf_AD(F_AD(stats), F_HP(stats), F_HP2AD(stats));
	crit = llf_crit(F_CRIT_BONUS(stats), F_CRIT_CHANCE(stats));
	aspd = llf_attackspeed(cfg->level, F_ATTACK_SPEED(stats));

	output[id][0] = llf_dmgtotal(cfg, &stats);
	output[id][1] = llf_autoattack_DPS(mit, ad, cfg->level, &stats);
	output[id][2] = mit;
	output[id][3] = ad;
	output[id][4] = crit;
	output[id][5] = aspd;
	output[id][6] = llf_ability_damage(mit, ad, cfg->ad_ratio);
}


#endif


