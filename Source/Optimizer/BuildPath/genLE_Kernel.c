#ifndef __OPENCL_VERSION__ 
#include "../Common/OpenCLDebugHack.h"
#endif

#include "le_kernel.h"

#include "../Common/ll_formulas.h"
#include "../../Database/db_layout.h"
#define COMBO_WIDTH_MAX 64


void print_le(struct string_ctx s);



static 
unsigned char
set_difference(mask_t a, mask_t b, int le_n) // PORTING: possible endian issues
{
	int j;
	unsigned char ideal = 0;
	mask_t mask = a & ~b;
	

	for (j = 0; j < le_n; j++)
	{
		if (1<<j & mask)
		{
			ideal = j+1;
			break;
		}
	}

	return ideal;
}



static
unsigned char
extend(mask_t a, mask_t b, struct string_ctx *s)
{
	unsigned char ideal;
	
	if (a == b)
		return 0;

	ideal = set_difference(a, b, s->s_len);

	if (ideal != 0 && s->s_index < s->s_len)
	{
		s->s_ptr[s->s_index] = ideal;
		s->s_index++;
	}
	return ideal;
}



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



float metricAreaDPS(
	__constant float items[][ITEM_WIDTH],
	__constant float passives[][PASSIVE_WIDTH],
	__constant float *cfg,
	ushort *ids,
	uint    combo_len)
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


#define GET_NEIGHBORS(P,S,I) &P[I*S]


int linear_extension(
	__constant mask_t *masks, 
	__constant ulong  *counts,
	__constant uchar  *adjacency,
	uint  max_neighbors,
	ulong nth_extension, 
	struct string_ctx *le)
{
	mask_t prev_mask;
	//uchar neighbors[NEIGHBOR_LIMIT];
	__constant uchar *neighbors;

	size_t i;
	int node_index = 1; // TODO: make sure this is a valid assumption

	prev_mask = masks[node_index];
	

	while (le->s_index < le->s_len)
	{
		ulong interval[2];
		ulong prev_count;

		neighbors = GET_NEIGHBORS(adjacency, max_neighbors, node_index);
		//for (i = 0; i < max_neighbors; ++i)
		//	neighbors[i] = adjacency[(node_index*max_neighbors)+i];

		for (prev_count = 0, node_index = -1, i = 0; i < max_neighbors; ++i)
		{
			if (neighbors[i] == 0)
				continue; // TODO: break instead?

			//next = GET_VERTEX(vertex, sizeof(struct vertex), node->neighbors_out[i]);
			node_index = neighbors[i];

			interval[0] = prev_count;
			prev_count += counts[node_index];
			interval[1] = prev_count - 1;

			if (nth_extension >= interval[0] && nth_extension <= interval[1])
				break;
			else
				node_index = -1;
		}

		if (node_index == -1) // error, nothing found
			return 1;

		extend(masks[node_index], prev_mask, le);

		prev_mask = masks[node_index];
		nth_extension -= interval[0];
	}

	return 0;
}



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



__kernel void kernel_LE(
	__constant float items[][ITEM_WIDTH],
	__constant float passives[][PASSIVE_WIDTH],
	__constant float *cfg,
	__constant ushort node2id[],

	__constant mask_t *masks,
	__constant ulong  *counts,
	__constant uchar  *adjacency,
	uint max_neighbors,
	uint combo_len,

	__local result_t* scratch,
	__global result_t* result)
{
	size_t nth_extension = get_global_id(0);
	struct string_ctx le;
	uchar le_buf[COMBO_WIDTH_MAX];
	ushort ids[COMBO_WIDTH_MAX];
	uint i;
	result_t metric;

	le.s_index = 0;
	le.s_len   = combo_len;
	le.s_ptr   = le_buf;

	i = linear_extension(masks, counts, adjacency, max_neighbors, (ulong) nth_extension, &le);
	//result[nth_extension].index = i;// vertex[nth_extension].count;// sizeof(struct vertex);
	//result[nth_extension].metric = le_buf[4];//vertex[nth_extension].mask;//le_buf[4];//adjacency[nth_extension][1];
	//result[nth_extension].index = adjacency[(nth_extension*3)+1];
	//result[nth_extension].metric = adjacency[nth_extension*3];
	
	for (i = 0; i < combo_len; i++)
		ids[i] = node2id[le.s_ptr[i] - 1] - 1; // convert to zero based index

	metric.metric = metricAreaDPS(items, passives, cfg, ids, combo_len);
	metric.index  = nth_extension;
	
	//result[nth_extension] = metric;
	


	//DEBUG_PRINTF("%lu\n", (unsigned int)nth_extension);
	//print_le(le);
	//DEBUG_PRINTF("items={%d,%d,%d,%d,%d,%d}\n", ids[0], ids[1], ids[2], ids[3], ids[4], ids[5]);


	reduce(metric, scratch, result);
}







