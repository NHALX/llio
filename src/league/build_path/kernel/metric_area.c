#ifndef __OPENCL_VERSION__ 
#include "opencl_host/dummy.h"
#include <limits.h>
#endif

#include "types.h"
#include "league/database/db_layout.h"
#include "league/ll_formulas.h"
#include "league/build_path/kernel/metric_area.h"

result_t RateBuildpath(__global ideal_t *linext, DB, llf_criteria *cfg, __local ideal_t *pasv_scratch, lattice_info *info, count_t nth_extension);
////////////////////////////////////////////////////////
#define PASV_SCRATCH_LEN(X) (X+1)

#define ZERO_INIT(BUF, N) do { \
for (size_t zi = 0; zi < N; ++zi)\
	BUF[zi] = 0; \
} while (0)




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
        stats_remove(db_items, stats, id, --unique[passive_id] == 0);
		++removed_n;
	}

	passive_id = db_items[item].passive_id;
	stats_add(db_items, stats, item, ++unique[passive_id] == 1);
	return 1 - removed_n;
}


// ASSUMES: item passive_id's arent larger than LINEXT_WIDTH_MAX and 
// that there is less than LINEXT_WIDTH_MAX passives being used 

static
int
MetricArea(DB,
	llf_criteria *cfg,
	itemid_t *dbi,
	uint_t dbi_n,
	__local ideal_t *unique,
	float *output)
{
	size_t bs;
	uint_t inventory_slots;
	float_t result   = 0;
	float_t prev_dps = 0;
    //uint total_cost = 0; 

	VECTOR(stats) = VECTOR_ZERO_INIT;
  
	for (bs = 0, inventory_slots = 0; bs < dbi_n; ++bs) 
	{	
		size_t item = dbi[bs];

		inventory_slots += AddItem(db_items, unique, (&stats), item);
		
		if (inventory_slots > cfg->build_maxinventory) // invalid build path
			{ *output = 0; return ERROR_INVENTORY; }

		result += (prev_dps * ((float)db_items[dbi[bs]].upgrade_cost));
        //total_cost += db_items[dbi[bs]].upgrade_cost;
        prev_dps = llf_metric(cfg, &stats);
	}
    *output = result;
    //*output = (total_cost) ? result / total_cost : 0;
	return ERROR_NONE;
}

#ifdef __OPENCL_VERSION__
#include "opencl_host/kernel/reduce.cl"

__kernel void metric_area(
    ulong_t linext_offset,
    lattice_info info,
    __global ideal_t *linext,
    DB,
    llf_criteria cfg,
    __local ideal_t *pasv_scratch,
    __local result_t* scratch,
    __global result_t* result)
{
    result_t metric = { 0, 0 };
    count_t nth_extension = linext_offset + get_global_id(0);

    linext       += info.linext_width * get_global_id(0);
    pasv_scratch += PASV_SCRATCH_LEN(info.linext_width) * get_local_id(0);

    ZERO_INIT(pasv_scratch, PASV_SCRATCH_LEN(info.linext_width));

    metric = RateBuildpath(linext, db_items, &cfg, pasv_scratch, &info, nth_extension);
    Reduce(metric, scratch, result);
}

#else
#include <string.h>
#ifndef _MSC_VER
#include <alloca.h>
#else
#define alloca _alloca
#endif

opencl_allocinfo
metric_area__allocnfo__(opencl_function *func, size_t items_n)
{
    cl_ulong klmem;
    opencl_allocinfo nfo = { 0 };

    NOFAIL(clGetKernelWorkGroupInfo(func->kernel, func->ctx->device,
        CL_KERNEL_LOCAL_MEM_SIZE, sizeof klmem, &klmem, 0));

    nfo.fixed.constant += sizeof (item_t) * items_n;

    nfo.scale_workgroup.local += sizeof (ideal_t) * items_n;
    nfo.scale_workgroup.local += sizeof (result_t);
    nfo.scale_workgroup.local += klmem;

    nfo.scale_reduce.global = sizeof (result_t);
    return nfo;
}

opencl_kernel_arg *
metric_area__bind__(opencl_function *X, bool_t copy_output,
    lattice_info *info,
    opencl_kernel_arg *linext,
    item_t *items, size_t items_n,
    llf_criteria *cfg,    
    size_t pass_size, 
    size_t local_size)
{
    size_t outlen;

    ka_ignore(X);
    ka_value(X, "lattice_info", info, sizeof *info);
    ka_reuse(X, linext);
    ka_mconst(X, "db_items", 0, items, sizeof *items * items_n);
    ka_value(X,  "cfg_input", cfg, sizeof *cfg);
    ka_mlocal(X, "pasv_scratch", sizeof (ideal_t) * items_n * local_size);
    ka_mlocal(X, "scratch", sizeof (result_t) * local_size);

    outlen = pass_size / local_size;
    // NOTE: this CL_MEM_WRITE_ONLY hint should be changed if this function is composed with another after it
    return ka_mglobal(X, "output", copy_output ? A_OUT : 0, CL_MEM_WRITE_ONLY, 0, sizeof(result_t)* outlen);
}

#undef X
 
result_t metric_area(__global ideal_t *linext, DB, llf_criteria *cfg, lattice_info *info, count_t nth_extension)
{
    const size_t size = sizeof (ideal_t) * PASV_SCRATCH_LEN(info->linext_width);
    ideal_t *pasv_scratch = alloca(size);
    memset(pasv_scratch, 0, size);

    return RateBuildpath(linext, db_items, cfg, pasv_scratch, info, nth_extension);
}

#endif


result_t 
RateBuildpath(__global ideal_t *linext, DB, llf_criteria *cfg, __local ideal_t *pasv_scratch, 
    lattice_info *info, count_t nth_extension)
{
	result_t metric;
	
	metric.metric = 0;
	metric.index  = ERROR_IGNORED;

    if (nth_extension < info->linext_count && linext[0] != 0)
	{
		itemid_t ids[LINEXT_WIDTH_MAX]; 
		size_t i;
		int error;

        // TODO: get rid of this
		// NOTE: We dont actually need the extra precision here, but if we get rid of it
		// some of the support functions won't be able to index the full database.
		for (i = 0; i < info->linext_width; i++)
            ids[i] = linext[i];
		
		error = MetricArea(db_items, cfg, ids, info->linext_width, pasv_scratch, &metric.metric);	
		metric.index = (error != ERROR_NONE) ? error : nth_extension;
	}

	return metric;
}

