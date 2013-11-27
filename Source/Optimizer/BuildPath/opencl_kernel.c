#ifndef __OPENCL_VERSION__ 
#include "../Common/OpenCLDebugHack.h"
#include <limits.h>
#endif

#include "le_kernel.h"

#include "../../Database/db_layout.h"
#include "../Common/ll_formulas.h"

////////////////////////////////////////////////////////
  

#define DB __constant item_t db_items[]

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

inline void
PassiveUnique(DB,
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
		uint passive_id = db_items[dbi[bs]].passive_id;

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
ClearSubComponents(DB,
	itemid_t *dbi,
	ushort2 unique[],
	ideal_t unique_i[],
	VECTOR(*stats),
	size_t build_step)
{
	index_t i;
	int removed_n = 0;

	for (i = 0; i < BUILDTREE_WIDTH; ++i)
	{
		index_t id = db_items[dbi[build_step]].buildtree[i];

		for (size_t k = 0; k < build_step; ++k)
		{
			// NOTE: This assumes the 0th db entry to be a null item
			if (dbi[k] != 0 && dbi[k] == id)
			{
				--unique[unique_i[k]].y;
				RemoveStats(db_items, stats, dbi[k], unique[unique_i[k]].y == 0);
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
MetricAreaDPS(DB,
	llf_criteria *cfg,
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
	//stats_t stats  = (stats_t){ 0, 0, 0, 0, 0, 0, 0, 0 };
	//c_short stats[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
	VECTOR(stats) = VECTOR_ZERO_INIT;

	PassiveUnique(db_items, dbi, dbi_n, unique, unique_i);
	
	for (bs = 0, inventory_slots = 0; bs < dbi_n; ++bs) // TODO: This can be optimized
	{	
		inventory_slots += 1 - ClearSubComponents(db_items, dbi, unique, unique_i, (&stats), bs);
		
		if (inventory_slots > cfg->build_maxinventory) // invalid build path
			{ *output = 0; return ERROR_INVENTORY; }
	
		unique[unique_i[bs]].y++; // reference count
		MergeStats(db_items, &stats, dbi[bs], unique[unique_i[bs]].y == 1);

		result += (prev_dps * (float) db_items[dbi[bs]].upgrade_cost);
		prev_dps = llf_dmgtotal(cfg, &stats); // FORMULA_TOTAL(cfg, stats); 
	}
	 
	*output = result;
	return ERROR_NONE;
}


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
result_t k_buildpath(DB,
	llf_criteria *cfg,
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

		
		error = MetricAreaDPS(db_items, cfg, ids, info.linext_width, &metric.metric);	
		metric.index = (error != ERROR_NONE) ? error : nth_extension;
	}

	return metric;
}


#ifdef __OPENCL_VERSION__

__kernel void kernel_buildpath(DB,
	llf_criteria cfg,
	__constant itemid_t node2id[],

	__global ideal_t *ideals,
	__global count_t *counts,
	__global index_t *adjacency,
	struct buildpath_info info,

	__local result_t* scratch,
	__global result_t* result)
{
	result_t metric = k_buildpath(db_items, &cfg, node2id, ideals, counts, adjacency, info, get_global_id(0));
	Reduce(metric, scratch, result);
}

	//////////////////////// tests /////////////////////////
	 
#ifdef UNIT_TEST

__kernel void kunittest_mem(DB,
	__global itemid_t id[],
	__global ushort  stat_s0[],
	__global ushort  stat_s3[],
	__global ushort  stat_s6[],
	__global VECTOR(*stat_all))
{
	size_t i = get_global_id(0);
	id[i] = db_items[i].id;
	stat_s0[i] = PART(db_items[i].stats, 0);
	stat_s3[i] = PART(db_items[i].stats, 3);
	stat_s6[i] = PART(db_items[i].stats, 6);

	VECTOR_COPY(stat_all[i], db_items[i].stats);
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


__kernel void kunittest_mergestats(DB,
	__global itemid_t *xs,
	uint xs_n,
	__global VECTOR(*output))
{
	VECTOR(stats) = VECTOR_ZERO_INIT;
	size_t i;
	size_t id = get_global_id(0);

	xs += id * xs_n;

	for (i = 0; i < xs_n; ++i)
		MergeStats(db_items, &stats, xs[i], true);

	VECTOR_COPY(output[id], stats);
}


__kernel void kunittest_passiveuniqe(DB,
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

	PassiveUnique(db_items, xs, xs_n, unique, unique_i);

	for (i = 0; i < xs_n; ++i)
	{
		output[id][i] = unique[i];
		output_idx[id][i] = unique_i[i];
	}

	for (i = xs_n; i < LINEXT_WIDTH_MAX; ++i)
	{
		output[id][i] = (ushort2){ 0, 0 };
		output_idx[id][i] = 0;
	}
}



__kernel void kunittest_clearsubcomponents(DB,
	__global itemid_t *xs_in,
	uint xs_n,
	__global int(*output_inventory)[LINEXT_WIDTH_MAX],
	__global VECTOR(*output_stats))
{
	size_t x;
	int inventory_slots;
	ushort2 unique[LINEXT_WIDTH_MAX];
	ideal_t unique_i[LINEXT_WIDTH_MAX];
	itemid_t xs[LINEXT_WIDTH_MAX];
	VECTOR(stats) = VECTOR_ZERO_INIT;
	size_t id = get_global_id(0);

	xs_in += id * xs_n;
	for (x = 0; x < xs_n; ++x)
		xs[x] = xs_in[x];

	PassiveUnique(db_items, xs, xs_n, unique, unique_i);

	for (x = 0, inventory_slots = 0; x < xs_n; ++x)
	{
		inventory_slots += 1 - ClearSubComponents(db_items, xs, unique, unique_i, &stats, x);
		unique[unique_i[x]].y++; // reference count
		MergeStats(db_items, &stats, xs[x], unique[unique_i[x]].y == 1);

		output_inventory[id][x] = inventory_slots;
	}

	for (x = xs_n; x < LINEXT_WIDTH_MAX; ++x)
		output_inventory[id][x] = 0;

	VECTOR_COPY(output_stats[id], stats);
}

 
__kernel void kunittest_llformulas(DB,
	__global itemid_t *xs,
	llf_criteria cfg,
	__global float(*output)[7])
{
	size_t id = get_global_id(0);
	VECTOR(stats);
	float mit, ad, crit, aspd;

	VECTOR_COPY(stats, db_items[xs[id]].stats);

	mit = llf_armor_mitigation(cfg.enemy_armor, F_ARMORPEN_PERCENT(stats), F_ARMORPEN_FLAT(stats));
	ad = llf_AD(F_AD(stats), F_HP(stats), F_HP2AD(stats));
	crit = llf_crit(F_CRIT_BONUS(stats), F_CRIT_CHANCE(stats));
	aspd = llf_attackspeed(cfg.level, F_ATTACK_SPEED(stats));

	output[id][0] = llf_dmgtotal(&cfg, &stats);
	output[id][1] = llf_autoattack_DPS(mit, ad, cfg.level, &stats);
	output[id][2] = mit;
	output[id][3] = ad;
	output[id][4] = crit;
	output[id][5] = aspd;
	output[id][6] = llf_ability_damage(mit, ad, cfg.ad_ratio);
}

#endif

#endif


//////////////////////////// Kernel  ////////////////////////////
