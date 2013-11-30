#include "kernel_LMR.c"
#include "reduce.cl"

__kernel void kernel_reduce(
	__global result_t  *metric,
	__local result_t   *scratch,
	__global result_t  *result)
{
	Reduce(metric[get_global_id(0)], scratch, result);
}
  

__kernel void kernel_metric(DB,
	llf_criteria       cfg,
	__local ideal_t    *pasv_scratch,

	__global ideal_t   *linext,
	ideal_t             linext_width,
	
	__global result_t  *output)
{
	result_t metric;
	itemid_t ids[LINEXT_WIDTH_MAX];
	size_t i;
	int error;
	
	linext += linext_width * get_global_id(0);
	pasv_scratch += PASV_SCRATCH_LEN(linext_width) * get_local_id(0);
	ZERO_INIT(pasv_scratch, PASV_SCRATCH_LEN(linext_width));

	if (linext[0] == 0)
	{
		metric.metric = 0;
		metric.index = ERROR_IGNORED;
	}
	else
	{
		// NOTE: We dont actually need the extra precision here, but if we get rid of it
		// some of the support functions won't be able to index the full database.
		for (i = 0; i < linext_width; i++)
			ids[i] = linext[i];

		error = MetricAreaDPS(db_items, &cfg, ids, linext_width, pasv_scratch, &metric.metric);
		metric.index = (error != ERROR_NONE) ? error : get_global_id(0);
	}

	output[get_global_id(0)] = metric;	
}


__kernel void kernel_linext(
	__global ideal_t *ideals,
	__global count_t *counts,
	__global index_t *adjacency,
	buildpath_info info,
	__global ideal_t *output)
{
	count_t nth_extension;
	size_t i;

	nth_extension = info.linext_offset + get_global_id(0);
	output += info.linext_width *  get_global_id(0);
	
	if (nth_extension < info.linext_count)
	{
		linext_t le;

		le.le_index = info.linext_width;
		le.le_len = info.linext_width;

		k_linext_nth(ideals, counts, adjacency, info.max_neighbors, nth_extension, &le);

		for (i = 0; i < info.linext_width; i++)
			output[i] = le.le_buf[i];
	}
	else 
	{
		for (i = 0; i < info.linext_width; i++)
			output[i] = 0;
	}
}

