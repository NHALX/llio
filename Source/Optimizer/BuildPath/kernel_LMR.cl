#include "kernel_LMR.c"
#include "reduce.cl"
  
__kernel void kernel_LMR(
    __global ideal_t *ideals,
    __global count_t *counts,
    __global index_t *adjacency,
    buildpath_info info,

    DB,
    llf_criteria cfg,
    __local ideal_t *pasv_scratch,

    __local result_t* scratch,
    __global result_t* result)
{
    result_t metric = k_buildpath(db_items, &cfg, pasv_scratch, ideals, counts, adjacency, &info, get_global_id(0), get_local_id(0));
    Reduce(metric, scratch, result);
}

  