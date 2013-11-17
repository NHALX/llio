#ifndef __OPENCL_VERSION__ 
#include "../Common/OpenCLDebugHack.h"
#include <limits.h>
#endif

#include "le_kernel.h"

#include "../Common/ll_formulas.h"
#include "../../Database/db_layout.h"
#define COMBO_WIDTH_MAX 64

//////////////////////////// Debug  ////////////////////////////

void print_le(struct string_ctx s);

void
print_le(struct string_ctx s)
{
	unsigned int i;

	DEBUG_PRINTF("%c", '{');
	for (i = 0; i < s.s_len; ++i)
	{
		DEBUG_PRINTF("%d", s.s_ptr[i]);
		
		if (i+1 < s.s_len)
			DEBUG_PRINTF("%c", ',');
	}

	DEBUG_PRINTF("%c\n", '}');
}


//////////////////////////// Metrics  ////////////////////////////

float metricAreaDPS(
	__constant float items[][ITEM_WIDTH],
	__constant float passives[][PASSIVE_WIDTH],
	__constant float *cfg,
	itemid_t *ids,
	uint combo_len)
{
	int passive_unique[COMBO_WIDTH_MAX]; // must be less than or eq combo_len
	float metric = 0;
	int i, passive_index = 0;
	uint id_i;

	float merged[ITEM_WIDTH];
	
	for (i = 0; i < ITEM_WIDTH; ++i) // initialize
		merged[i] = 0;


	for (id_i = 0; id_i < combo_len; ++id_i)
	{
		int id = ids[id_i];
		int ip = (int)items[id][F_PASSIVE];

		for (i = 0; i < passive_index; ++i)
		{
			if (passive_unique[i] == ip)
				break;
		}

		if (i == passive_index)
		{
			passive_unique[passive_index++] = ip;

			for (i = 0; i < PASSIVE_WIDTH; ++i)
				merged[i + ITEM_PASSIVE_SYNC_OFFSET] += passives[ip][i]; // TODO: handle stats that are merged multiplicatively
		}

		for (i = 0; i < ITEM_WIDTH; ++i) // merge item
			merged[i] += items[id][i]; // TODO: handle stats that are merged multiplicatively

		metric += FORMULA_TOTAL(cfg, merged) * items[id][F_UPGRADE_COST];

		//DEBUG_PRINTF("dps=%f\n", FORMULA_TOTAL(cfg, merged));
		//DEBUG_PRINTF("cost=%f, metric=%f\n", items[id][F_UPGRADE_COST], metric / 1000);
	}
	/*
	DEBUG_PRINTF("merged={%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f}\n",
		merged[0], merged[1], merged[2], merged[3],
		merged[4], merged[5], merged[6], merged[7],
		merged[8], merged[9], merged[10], merged[11]);
	*/

	return metric / 1000;
}

//////////////////////////// Generate linear extension  ////////////////////////////

#define GET_EDGES(P,S,I) (&P[I*S])

static
unsigned char
extend(unsigned char ideal, struct string_ctx *s)
{
	if (ideal != 0 && s->s_index > 0)
	{
		s->s_index--;
		s->s_ptr[s->s_index] = ideal;
	}
	return ideal;
}

int linear_extension(
	__constant ideal_t *ideals,
	__constant count_t *counts,
	__constant index_t *adjacency,
	uint  max_neighbors,
	count_t nth_extension,
	struct string_ctx *le)
{
	__constant index_t *neighbors;
	__constant ideal_t *iedges;
	size_t i;
	int node_index = 0; // TODO: make sure this is a valid assumption


	while (le->s_index > 0)
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

		extend(ideal, le);
		nth_extension -= interval[0];
	}

	return 0;
}



//////////////////////////// Find maximum  ////////////////////////////

void reduce(result_t dps, __local result_t* scratch, __global result_t* result)
{
	int offset;
	int local_index;

	// Perform parallel reduction
	local_index = get_local_id(0);
	scratch[local_index] = dps;
	barrier(CLK_LOCAL_MEM_FENCE);

	for (offset = get_local_size(0) / 2;
		offset > 0;
		offset = offset / 2)
	{
		if (local_index < offset) {
			result_t other = scratch[local_index + offset];
			result_t mine = scratch[local_index];
			scratch[local_index] = (mine.metric > other.metric) ? mine : other;
		}
		barrier(CLK_LOCAL_MEM_FENCE);
	}

	if (local_index == 0) {
		result[get_group_id(0)] = scratch[0];
	}
}


//////////////////////////// Kernel  ////////////////////////////

__kernel void kernel_LE(
	__constant float items[][ITEM_WIDTH],
	__constant float passives[][PASSIVE_WIDTH],
	__constant float *cfg,
	__constant itemid_t node2id[],

	__constant ideal_t *ideals,
	__constant count_t *counts,
	__constant index_t *adjacency,
	uint max_neighbors,
	uint combo_len,

	__local result_t* scratch,
	__global result_t* result)
{
	size_t nth_extension = get_global_id(0);
	struct string_ctx le;
	ideal_t le_buf[COMBO_WIDTH_MAX];
	itemid_t ids[COMBO_WIDTH_MAX];
	uint i;
	result_t metric;

	le.s_index = combo_len;
	le.s_len   = combo_len;
	le.s_ptr   = le_buf;

	// TODO: the size_t precision of nth_extension is a problem.
	// the system wont be able to queue the full workload anyway
	// so an offset should be added later to split the work up 

	i = linear_extension(ideals, counts, adjacency, max_neighbors, (count_t) nth_extension, &le);
	//result[nth_extension].index = i;// vertex[nth_extension].count;// sizeof(struct vertex);
	//result[nth_extension].metric = le_buf[4];//vertex[nth_extension].mask;//le_buf[4];//adjacency[nth_extension][1];
	//result[nth_extension].index = adjacency[(nth_extension*3)+1];
	//result[nth_extension].metric = adjacency[nth_extension*3];
	
	for (i = 0; i < combo_len; i++)
		ids[i] = node2id[le.s_ptr[i] - 1]; // convert to zero based index

	metric.metric = metricAreaDPS(items, passives, cfg, ids, combo_len);
	metric.index  = nth_extension;
	
	//result[nth_extension] = metric;
	


	//DEBUG_PRINTF("%lu\n", (unsigned int)nth_extension);
	//print_le(le);
	//DEBUG_PRINTF("items={%d,%d,%d,%d,%d,%d}\n", ids[0], ids[1], ids[2], ids[3], ids[4], ids[5]);


	reduce(metric, scratch, result);
}







