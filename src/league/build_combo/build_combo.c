#ifndef __OPENCL_VERSION__ 
#include "../../opencl_host/dummy.h"
#include <stdlib.h>
#undef DEBUG_PRINTF
#define DEBUG_PRINTF
#else
#define DEBUG_PRINTF 
#endif

#include "types.h"
#include "league/database/db_layout.h"
#include "league/ll_formulas.h"
#include "league/common.h"
#include "combinatorics/combination.h"


static
int
metric(__constant item_t db_items[],
	llf_criteria *cfg,
    setmax_t *combo,
	uint_t combo_n,
    __local setmax_t *unique,
	float *output)
{
	size_t i;

	VECTOR(stats) = VECTOR_ZERO_INIT;
  
    for (i = 0; i < combo_n; ++i)
    {
        itemid_t item = combo[i];
        itemid_t passive = db_items[item].passive_id;

        stats_add(db_items, &stats, item, ++unique[passive] == 1);
    }

    *output = llf_dmgtotal(cfg, &stats);
    return 0;
}


//////////////////////////////////////////////////////////////


result_t
BuildCombo(__constant item_t db_items[],
    llf_criteria *cfg,
    ulong_t rank,
    uint_t db_len,
    uint_t combo_len,
    __local setmax_t *unique)
{
    setmax_t combo[NK_MULTISET_MAX_K];
    result_t dps;

    combo_unrank(rank, db_len, combo_len, combo);
    dps.index = rank;
    metric(db_items, cfg, combo, combo_len, unique, &dps.metric);
    return dps;
}


#ifndef __OPENCL_VERSION__ 
#include "opencl_host/function.h"
opencl_allocinfo
build_combo__allocnfo__(opencl_function *func, size_t items_n)
{
    cl_ulong klmem;
    opencl_allocinfo nfo = { 0 };

    NOFAIL(clGetKernelWorkGroupInfo(func->kernel, func->ctx->device,
        CL_KERNEL_LOCAL_MEM_SIZE, sizeof klmem, &klmem, 0));
    /*TODO: fill these out
    nfo.fixed.constant += sizeof (item_t)* items_n;

    nfo.scale_workgroup.local += sizeof (ideal_t)* items_n;
    nfo.scale_workgroup.local += sizeof (result_t);
    nfo.scale_workgroup.local += klmem;

    nfo.scale_reduce.global = sizeof (result_t);*/
    return nfo;
}

opencl_kernel_arg *
build_combo__bind__(opencl_function *X,
    uint_t combo_n,
    item_t *items, uint_t items_n,
    llf_criteria *cfg,
    
    size_t pass_size, 
    size_t local_size)
{
    size_t outlen;
    
    ka_ignore(X);
    ka_mconst(X, "db_items", 0, items, sizeof *items * items_n);
    ka_value(X,  "cfg_input", cfg, sizeof *cfg);
    ka_value(X, "db_len", &items_n, sizeof items_n);
    ka_value(X, "combo_len", &combo_n, sizeof combo_n);
    ka_mlocal(X, "pasv_scratch", sizeof (setmax_t) * items_n * local_size);
    ka_mlocal(X, "scratch", sizeof (result_t) * local_size);

    outlen = pass_size / local_size;
    return ka_mglobal(X, "output", A_OUT, CL_MEM_WRITE_ONLY, 0, sizeof(result_t)* outlen);
}

#else
#include "opencl_host/kernel/reduce.cl"
#include "combinatorics/combination.c"
#include "combinatorics/factorial.c"

__kernel void
build_combo(
    ulong_t start_offset, 
    __constant item_t db_items[], 
    llf_criteria cfg,
    uint_t db_len, 
    uint_t combo_len,
    __local setmax_t *unique,
    __local result_t *scratch,
    __global result_t *result)
{
    result_t dps = BuildCombo(db_items, &cfg, start_offset + get_global_id(0), db_len, combo_len, unique);
    Reduce(dps, scratch, result);
}    

#endif
